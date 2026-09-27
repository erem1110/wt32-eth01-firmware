#include "relay_subsystem.h"
#include "storage.h"
#include "hardware_io.h"
#include "emergency_subsystem.h"
#include "timer_manager.h"
#include "event_log.h"
#include "mqtt_manager.h"
#include "web_server.h"

RelaySubsystem Relays;

void onAutoOffExpired(uint8_t relay_idx, uint32_t epoch) {
    Relays.handleAutoOffExpired(relay_idx, epoch);
}

RelaySubsystem::RelaySubsystem() : last_energy_save_ms(0) {
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        runtimes[i].state = false;
        runtimes[i].target_state = false;
        runtimes[i].last_saved_state = false;
        runtimes[i].timer_epoch = 0;
        runtimes[i].auto_off_expire_ms = 0;
        runtimes[i].pending_state = RELAY_IDLE;
        runtimes[i].delay_expire_ms = 0;
        runtimes[i].pending_source = SRC_SYSTEM;
        runtimes[i].current_on_start_ms = 0;
        runtimes[i].energy.total_cycles = 0;
        runtimes[i].energy.total_on_seconds = 0;
    }
}

void RelaySubsystem::begin() {
    bool emg_locked = Emergency.isRelaysLocked();
    last_energy_save_ms = millis();

    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        bool last_st = false;
        Storage.loadRelay(i, configs[i], last_st);
        Storage.loadRelayEnergy(i, runtimes[i].energy);
        runtimes[i].last_saved_state = last_st;
        runtimes[i].current_on_start_ms = 0;

        if (!emg_locked && configs[i].enabled) {

            if (configs[i].restore_mode == RESTORE_ALWAYS_ON) {
                turnRelayOn(i, SRC_SYSTEM);
            } else if (configs[i].restore_mode == RESTORE_PREVIOUS && last_st) {
                turnRelayOn(i, SRC_SYSTEM);
            } else {
                HW.setRelayPhysical(i, false);
            }
        } else {
            HW.setRelayPhysical(i, false);
        }
    }
    HW.writeRelayOutputs();
}

bool RelaySubsystem::turnRelayOn(uint8_t index, CommandSource source) {
    if (index >= NUM_RELAYS) return false;

    if (Emergency.isRelaysLocked()) {
        EventLog.log(SUBSYS_RELAY, "%s: Blocked ON command (Emergency locked)", configs[index].name);
        return false;
    }

    if (!configs[index].enabled) {
        return false;
    }

    if (source == SRC_WEB && !configs[index].manual_control_enabled) return false;
    if (source == SRC_MQTT && !configs[index].mqtt_control_enabled) return false;
    if (source == SRC_INPUT && !configs[index].input_control_enabled) return false;

    uint8_t grp = configs[index].interlock_group;
    if (grp > 0 && grp <= NUM_INTERLOCK_GROUPS) {
        for (uint8_t j = 0; j < NUM_RELAYS; ++j) {
            if (j != index && configs[j].interlock_group == grp) {
                if (runtimes[j].state || runtimes[j].pending_state == RELAY_PENDING_ON) {
                    applyPhysicalOff(j, source);
                }
            }
        }
    }

    if (runtimes[index].pending_state == RELAY_PENDING_OFF) {
        runtimes[index].pending_state = RELAY_IDLE;
    }

    if (configs[index].on_delay_ms > 0) {
        runtimes[index].pending_state = RELAY_PENDING_ON;
        runtimes[index].delay_expire_ms = millis() + configs[index].on_delay_ms;
        runtimes[index].pending_source = source;
        return true;
    }

    applyPhysicalOn(index, source);
    return true;
}

bool RelaySubsystem::turnRelayOff(uint8_t index, CommandSource source) {
    if (index >= NUM_RELAYS) return false;

    if (source == SRC_WEB && !configs[index].manual_control_enabled) return false;
    if (source == SRC_MQTT && !configs[index].mqtt_control_enabled) return false;
    if (source == SRC_INPUT && !configs[index].input_control_enabled) return false;

    if (runtimes[index].pending_state == RELAY_PENDING_ON) {
        runtimes[index].pending_state = RELAY_IDLE;
    }

    if (configs[index].off_delay_ms > 0 && runtimes[index].state) {
        runtimes[index].pending_state = RELAY_PENDING_OFF;
        runtimes[index].delay_expire_ms = millis() + configs[index].off_delay_ms;
        runtimes[index].pending_source = source;
        return true;
    }

    applyPhysicalOff(index, source);
    return true;
}

bool RelaySubsystem::toggleRelay(uint8_t index, CommandSource source) {
    if (index >= NUM_RELAYS) return false;
    if (runtimes[index].state || runtimes[index].pending_state == RELAY_PENDING_ON) {
        return turnRelayOff(index, source);
    } else {
        return turnRelayOn(index, source);
    }
}

void RelaySubsystem::allRelaysOff(CommandSource source) {
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        turnRelayOff(i, source);
    }
}

void RelaySubsystem::allRelaysOn(CommandSource source) {
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        turnRelayOn(i, source);
    }
}

void RelaySubsystem::applyPhysicalOn(uint8_t index, CommandSource source) {
    runtimes[index].timer_epoch++;
    runtimes[index].state = true;
    runtimes[index].pending_state = RELAY_IDLE;
    runtimes[index].energy.total_cycles++;
    runtimes[index].current_on_start_ms = millis();

    HW.setRelayPhysical(index, true);
    HW.writeRelayOutputs();

    if (configs[index].restore_mode == RESTORE_PREVIOUS) {
        if (!runtimes[index].last_saved_state) {
            runtimes[index].last_saved_state = true;
            Storage.saveRelayLastState(index, true);
        }
    }

    if (configs[index].auto_off_sec > 0) {
        TimerMgr.scheduleAutoOff(index, runtimes[index].timer_epoch, configs[index].auto_off_sec);
    } else {
        TimerMgr.cancelAutoOff(index);
    }

    if (configs[index].event_logging_enabled) {
        EventLog.log(SUBSYS_RELAY, "%s ON (src: %u)", configs[index].name, (uint8_t)source);
    }

    Mqtt.publishRelayState(index, true);
    WebServerMgr.broadcastState();
}

void RelaySubsystem::applyPhysicalOff(uint8_t index, CommandSource source) {
    if (runtimes[index].state && runtimes[index].current_on_start_ms > 0) {
        unsigned long elapsed = (millis() - runtimes[index].current_on_start_ms) / 1000UL;
        runtimes[index].energy.total_on_seconds += elapsed;
        runtimes[index].current_on_start_ms = 0;
        Storage.saveRelayEnergy(index, runtimes[index].energy);
    }

    runtimes[index].state = false;
    runtimes[index].pending_state = RELAY_IDLE;
    TimerMgr.cancelAutoOff(index);

    HW.setRelayPhysical(index, false);
    HW.writeRelayOutputs();

    if (configs[index].restore_mode == RESTORE_PREVIOUS) {
        if (runtimes[index].last_saved_state) {
            runtimes[index].last_saved_state = false;
            Storage.saveRelayLastState(index, false);
        }
    }

    if (configs[index].event_logging_enabled) {
        EventLog.log(SUBSYS_RELAY, "%s OFF (src: %u)", configs[index].name, (uint8_t)source);
    }

    Mqtt.publishRelayState(index, false);
    WebServerMgr.broadcastState();
}

void RelaySubsystem::forceEmergencyShutoff(uint16_t target_mask) {
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        if (target_mask & (1 << i)) {
            if (runtimes[i].state && runtimes[i].current_on_start_ms > 0) {
                unsigned long elapsed = (millis() - runtimes[i].current_on_start_ms) / 1000UL;
                runtimes[i].energy.total_on_seconds += elapsed;
                runtimes[i].current_on_start_ms = 0;
                Storage.saveRelayEnergy(i, runtimes[i].energy);
            }
            runtimes[i].pending_state = RELAY_IDLE;
            runtimes[i].state = false;
            TimerMgr.cancelAutoOff(i);
            HW.setRelayPhysical(i, false);
            if (configs[i].restore_mode == RESTORE_PREVIOUS) {
                runtimes[i].last_saved_state = false;
                Storage.saveRelayLastState(i, false);
            }
            Mqtt.publishRelayState(i, false);
        }
    }
    HW.writeRelayOutputs();
}

void RelaySubsystem::handleAutoOffExpired(uint8_t index, uint32_t target_epoch) {
    if (index >= NUM_RELAYS) return;
    if (runtimes[index].timer_epoch == target_epoch && runtimes[index].state) {
        if (configs[index].event_logging_enabled) {
            EventLog.log(SUBSYS_RELAY, "%s auto-off expired", configs[index].name);
        }
        turnRelayOff(index, SRC_TIMER);
    }
}

void RelaySubsystem::updateConfig(uint8_t index, const RelayConfig &cfg) {
    if (index >= NUM_RELAYS) return;
    configs[index] = cfg;
    Storage.saveRelay(index, cfg);
    if (!configs[index].enabled && runtimes[index].state) {
        turnRelayOff(index, SRC_SYSTEM);
    }
    EventLog.log(SUBSYS_RELAY, "%s config updated", configs[index].name);
}

void RelaySubsystem::saveAllEnergyStats() {
    unsigned long now = millis();
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        if (runtimes[i].state && runtimes[i].current_on_start_ms > 0) {
            unsigned long elapsed = (now - runtimes[i].current_on_start_ms) / 1000UL;
            runtimes[i].energy.total_on_seconds += elapsed;
            runtimes[i].current_on_start_ms = now;
        }
        Storage.saveRelayEnergy(i, runtimes[i].energy);
    }
}

void RelaySubsystem::resetEnergyStats(uint8_t index) {
    if (index >= NUM_RELAYS) return;
    runtimes[index].energy.total_cycles = 0;
    runtimes[index].energy.total_on_seconds = 0;
    if (runtimes[index].state) {
        runtimes[index].current_on_start_ms = millis();
    }
    Storage.saveRelayEnergy(index, runtimes[index].energy);
    EventLog.log(SUBSYS_RELAY, "%s energy statistics reset", configs[index].name);
    WebServerMgr.broadcastState();
}

uint32_t RelaySubsystem::getRelayTotalOnSeconds(uint8_t index) const {
    if (index >= NUM_RELAYS) return 0;
    uint32_t total = runtimes[index].energy.total_on_seconds;
    if (runtimes[index].state && runtimes[index].current_on_start_ms > 0) {
        total += (millis() - runtimes[index].current_on_start_ms) / 1000UL;
    }
    return total;
}

float RelaySubsystem::getRelayKWh(uint8_t index) const {
    if (index >= NUM_RELAYS) return 0.0f;
    uint32_t sec = getRelayTotalOnSeconds(index);
    float hours = (float)sec / 3600.0f;
    float kw = (float)configs[index].rated_power_w / 1000.0f;
    return hours * kw;
}

float RelaySubsystem::getTotalSystemPowerW() const {
    float total_w = 0.0f;
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        if (runtimes[i].state) {
            total_w += configs[i].rated_power_w;
        }
    }
    return total_w;
}

float RelaySubsystem::getTotalSystemKWh() const {
    float total_kwh = 0.0f;
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        total_kwh += getRelayKWh(i);
    }
    return total_kwh;
}

void RelaySubsystem::update() {
    unsigned long now = millis();

    // Periodic energy save every 5 minutes
    if (now - last_energy_save_ms > 300000UL) {
        last_energy_save_ms = now;
        saveAllEnergyStats();
    }

    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        if (runtimes[i].pending_state == RELAY_PENDING_ON) {
            if ((long)(now - runtimes[i].delay_expire_ms) >= 0) {
                applyPhysicalOn(i, runtimes[i].pending_source);
            }
        } else if (runtimes[i].pending_state == RELAY_PENDING_OFF) {
            if ((long)(now - runtimes[i].delay_expire_ms) >= 0) {
                applyPhysicalOff(i, runtimes[i].pending_source);
            }
        }
    }
}

