#include "emergency_subsystem.h"
#include "storage.h"
#include "event_log.h"
#include "relay_subsystem.h"
#include "mqtt_manager.h"
#include "web_server.h"

EmergencySubsystem Emergency;

EmergencySubsystem::EmergencySubsystem() {
    runtime.active = false;
    runtime.locked = false;
    runtime.activated_ms = 0;
    runtime.trigger_source[0] = '\0';
}

void EmergencySubsystem::begin() {
    bool nvs_locked = false;
    Storage.loadEmergency(config, nvs_locked);
    runtime.locked = nvs_locked;
    runtime.active = nvs_locked;
    if (runtime.locked) {
        strncpy(runtime.trigger_source, "Boot (Lock Persisted)", sizeof(runtime.trigger_source));
        EventLog.log(SUBSYS_EMERGENCY, "Restored persisted emergency lock");
    }
}

void EmergencySubsystem::executeEmergencyActions() {
    uint16_t mask = config.turn_off_all ? 0xFFFF : config.target_relays_mask;
    Relays.forceEmergencyShutoff(mask);

    if (config.lock_relays) {
        runtime.locked = true;
        Storage.setEmergencyLock(true);
    }

    EventLog.log(SUBSYS_EMERGENCY, "Emergency activated by %s", runtime.trigger_source);
    Mqtt.publishEmergency(true);
    WebServerMgr.broadcastState();
}

void EmergencySubsystem::triggerEmergency(const char *source) {
    runtime.active = true;
    runtime.activated_ms = millis();
    if (source) {
        strncpy(runtime.trigger_source, source, sizeof(runtime.trigger_source) - 1);
        runtime.trigger_source[sizeof(runtime.trigger_source) - 1] = '\0';
    } else {
        strncpy(runtime.trigger_source, "Manual", sizeof(runtime.trigger_source));
    }
    executeEmergencyActions();
}

bool EmergencySubsystem::resetEmergency(bool confirmed) {
    if (!confirmed) return false;

    runtime.active = false;
    runtime.locked = false;
    runtime.trigger_source[0] = '\0';
    Storage.setEmergencyLock(false);

    EventLog.log(SUBSYS_EMERGENCY, "Emergency reset confirmed");
    Mqtt.publishEmergency(false);
    WebServerMgr.broadcastState();
    return true;
}

void EmergencySubsystem::updateConfig(const EmergencyConfig &cfg) {
    config = cfg;
    Storage.saveEmergency(config);
    EventLog.log(SUBSYS_EMERGENCY, "Emergency config updated");
}

void EmergencySubsystem::update() {
    if (runtime.active && !config.manual_reset_only && config.auto_reset_seconds > 0) {
        unsigned long elapsed = (millis() - runtime.activated_ms) / 1000UL;
        if (elapsed >= config.auto_reset_seconds) {
            EventLog.log(SUBSYS_EMERGENCY, "Emergency auto-reset expired");
            resetEmergency(true);
        }
    }
}
