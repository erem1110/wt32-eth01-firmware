#include "input_subsystem.h"
#include "storage.h"
#include "hardware_io.h"
#include "relay_subsystem.h"
#include "emergency_subsystem.h"
#include "scene_subsystem.h"
#include "event_log.h"
#include "mqtt_manager.h"
#include "web_server.h"

InputSubsystem Inputs;

InputSubsystem::InputSubsystem() : last_poll_ms(0) {
    for (uint8_t i = 0; i < NUM_INPUTS; ++i) {
        runtimes[i].state = IN_STATE_IDLE;
        runtimes[i].physical_raw = true;
        runtimes[i].logical_active = false;
        runtimes[i].prev_logical_active = false;
        runtimes[i].long_press_fired = false;
        runtimes[i].timeout_warned = false;
        runtimes[i].startup_locked = false;
        runtimes[i].state_timer_ms = 0;
        runtimes[i].press_start_ms = 0;
        strncpy(runtimes[i].last_event, "none", sizeof(runtimes[i].last_event));
    }
}

void InputSubsystem::begin() {
    Storage.loadInputDefaults(defaults);

    uint16_t raw_word = HW.readInputsRaw();

    for (uint8_t i = 0; i < NUM_INPUTS; ++i) {
        Storage.loadInput(i, configs[i]);

        bool raw_bit = (raw_word >> i) & 1;
        runtimes[i].physical_raw = raw_bit;
        bool is_active = (configs[i].polarity == ACTIVE_LOW) ? (!raw_bit) : (raw_bit);

        if (configs[i].startup == STARTUP_PROCESS) {
            runtimes[i].logical_active = is_active;
            runtimes[i].prev_logical_active = is_active;
            runtimes[i].startup_locked = false;
            if (is_active && configs[i].enabled) {
                if (configs[i].mode == MODE_MOMENTARY || configs[i].mode == MODE_STATE) {
                    Relays.turnRelayOn(configs[i].target_relay - 1, SRC_INPUT);
                } else if (configs[i].mode == MODE_EMERGENCY || configs[i].is_emergency) {
                    Emergency.triggerEmergency(configs[i].name);
                }
            }
        } else if (configs[i].startup == STARTUP_WAIT_CHANGE) {
            runtimes[i].startup_locked = true;
            runtimes[i].logical_active = is_active;
            runtimes[i].prev_logical_active = is_active;
        } else {
            // STARTUP_IGNORE
            runtimes[i].logical_active = is_active;
            runtimes[i].prev_logical_active = is_active;
            runtimes[i].startup_locked = false;
        }
    }
}

void InputSubsystem::executeAction(InputAction act, uint8_t target_relay, uint8_t target_scene, const char *source_name) {
    switch (act) {
        case ACT_RELAY_ON:
            if (target_relay >= 1 && target_relay <= NUM_RELAYS) {
                Relays.turnRelayOn(target_relay - 1, SRC_INPUT);
            }
            break;
        case ACT_RELAY_OFF:
            if (target_relay >= 1 && target_relay <= NUM_RELAYS) {
                Relays.turnRelayOff(target_relay - 1, SRC_INPUT);
            }
            break;
        case ACT_RELAY_TOGGLE:
            if (target_relay >= 1 && target_relay <= NUM_RELAYS) {
                Relays.toggleRelay(target_relay - 1, SRC_INPUT);
            }
            break;
        case ACT_ACTIVATE_SCENE:
            if (target_scene >= 1 && target_scene <= NUM_SCENES) {
                Scenes.activateScene(target_scene - 1);
            }
            break;
        case ACT_ALL_RELAYS_OFF:
            Relays.allRelaysOff(SRC_INPUT);
            break;
        case ACT_ALL_RELAYS_ON:
            Relays.allRelaysOn(SRC_INPUT);
            break;
        case ACT_EMERGENCY_ACTIVATE:
            Emergency.triggerEmergency(source_name);
            break;
        case ACT_NONE:
        default:
            break;
    }
}

void InputSubsystem::processInputFsm(uint8_t i, bool is_active) {
    unsigned long now = millis();
    InputConfig &cfg = configs[i];
    InputRuntime &rt = runtimes[i];

    if (rt.startup_locked) {
        if (is_active != rt.prev_logical_active) {
            rt.startup_locked = false;
            rt.prev_logical_active = is_active;
        } else {
            return;
        }
    }

    switch (rt.state) {
        case IN_STATE_IDLE:
            if (is_active) {
                rt.state = IN_STATE_DEBOUNCE_PRESS;
                rt.state_timer_ms = now;
            }
            break;

        case IN_STATE_DEBOUNCE_PRESS:
            // Wait for full debounce period before validating press
            if ((now - rt.state_timer_ms) >= cfg.debounce_ms) {
                if (is_active) {
                    rt.state = IN_STATE_PRESSED;
                    rt.logical_active = true;
                    rt.press_start_ms = now;
                    rt.long_press_fired = false;
                    rt.timeout_warned = false;

                    if (cfg.log_enabled) {
                        EventLog.log(SUBSYS_INPUT, "%s pressed", cfg.name);
                    }
                    if (cfg.mqtt_pub) {
                        Mqtt.publishInputState(i, true);
                    }

                    // Process PRESS action based on mode
                    if (cfg.mode == MODE_MOMENTARY || cfg.mode == MODE_STATE) {
                        // Level / Momentary: Turn ON immediately on active
                        if (cfg.target_relay >= 1 && cfg.target_relay <= NUM_RELAYS) {
                            Relays.turnRelayOn(cfg.target_relay - 1, SRC_INPUT);
                        }
                    } else if (cfg.mode == MODE_TOGGLE) {
                        // Standard Toggle Button: Toggle immediately on press
                        if (cfg.target_relay >= 1 && cfg.target_relay <= NUM_RELAYS) {
                            Relays.toggleRelay(cfg.target_relay - 1, SRC_INPUT);
                        }
                    } else if (cfg.mode == MODE_SCENE) {
                        // Scene Button: Activate scene immediately on press
                        Scenes.activateScene(cfg.target_scene - 1);
                    } else if (cfg.mode == MODE_EMERGENCY || cfg.is_emergency) {
                        Emergency.triggerEmergency(cfg.name);
                    }
                    // For MODE_MULTI_ACTION: Do not act on press; wait for click / hold / double-click.

                    WebServerMgr.broadcastState();
                } else {
                    // False trigger (glitch shorter than debounce_ms)
                    rt.state = IN_STATE_IDLE;
                }
            }
            break;

        case IN_STATE_PRESSED:
            // Signal timeout check
            if (cfg.signal_timeout_s > 0 && !rt.timeout_warned) {
                if ((now - rt.press_start_ms) >= ((unsigned long)cfg.signal_timeout_s * 1000UL)) {
                    rt.timeout_warned = true;
                    strncpy(rt.last_event, "timeout", sizeof(rt.last_event));
                    if (cfg.log_enabled) {
                        EventLog.log(SUBSYS_INPUT, "%s: Signal timeout warning", cfg.name);
                    }
                    if (cfg.timeout_action == TIMEOUT_EMERGENCY) {
                        Emergency.triggerEmergency(cfg.name);
                    } else if (cfg.timeout_action == TIMEOUT_AUTO_RELEASE) {
                        if (cfg.mode == MODE_MOMENTARY || cfg.mode == MODE_STATE) {
                            if (cfg.target_relay >= 1 && cfg.target_relay <= NUM_RELAYS) {
                                Relays.turnRelayOff(cfg.target_relay - 1, SRC_INPUT);
                            }
                        }
                    }
                    WebServerMgr.broadcastState();
                }
            }

            // Long press check: ONLY for MODE_MULTI_ACTION or MODE_TOGGLE with configured long_action
            // NEVER for MODE_STATE or MODE_MOMENTARY (holding a wall switch or momentary contact is normal)
            if ((cfg.mode == MODE_MULTI_ACTION || (cfg.mode == MODE_TOGGLE && cfg.long_action != ACT_NONE)) &&
                !rt.long_press_fired && (now - rt.press_start_ms) >= cfg.long_press_ms) {
                rt.long_press_fired = true;
                strncpy(rt.last_event, "long_press", sizeof(rt.last_event));
                if (cfg.log_enabled) {
                    EventLog.log(SUBSYS_INPUT, "%s: Long press", cfg.name);
                }
                if (cfg.mqtt_pub) {
                    Mqtt.publishInputEvent(i, "long_press");
                }
                executeAction(cfg.long_action, cfg.target_relay, cfg.target_scene, cfg.name);
                WebServerMgr.broadcastState();
            }

            // Detect release
            if (!is_active) {
                rt.state = IN_STATE_DEBOUNCE_RELEASE;
                rt.state_timer_ms = now;
            }
            break;

        case IN_STATE_DEBOUNCE_RELEASE:
            // Wait for full debounce period before validating release
            if ((now - rt.state_timer_ms) >= cfg.debounce_ms) {
                if (!is_active) {
                    rt.logical_active = false;

                    if (cfg.log_enabled) {
                        EventLog.log(SUBSYS_INPUT, "%s released", cfg.name);
                    }
                    if (cfg.mqtt_pub) {
                        Mqtt.publishInputState(i, false);
                    }

                    // Process RELEASE action based on mode
                    if (cfg.mode == MODE_MOMENTARY || cfg.mode == MODE_STATE) {
                        // Level / Momentary: Turn OFF when released!
                        if (cfg.target_relay >= 1 && cfg.target_relay <= NUM_RELAYS) {
                            Relays.turnRelayOff(cfg.target_relay - 1, SRC_INPUT);
                        }
                        // Immediately return to IDLE! NEVER wait for double click or short action!
                        rt.state = IN_STATE_IDLE;
                    } else if (cfg.mode == MODE_TOGGLE) {
                        // Standard Toggle: Already toggled on press. Return cleanly to IDLE.
                        rt.state = IN_STATE_IDLE;
                    } else if (cfg.mode == MODE_MULTI_ACTION) {
                        // Multi-action button:
                        if (rt.long_press_fired) {
                            // Long press was already triggered during hold; do not fire short press
                            rt.state = IN_STATE_IDLE;
                        } else {
                            // Quick release: Wait to see if a 2nd click arrives for double click!
                            rt.state = IN_STATE_WAIT_DOUBLE;
                            rt.state_timer_ms = now;
                        }
                    } else {
                        // All other modes (SCENE, EMERGENCY, DISABLED)
                        rt.state = IN_STATE_IDLE;
                    }

                    WebServerMgr.broadcastState();
                } else {
                    // False release (switch bounce back to active)
                    rt.state = IN_STATE_PRESSED;
                }
            }
            break;

        case IN_STATE_WAIT_DOUBLE:
            // Only MODE_MULTI_ACTION ever enters this state!
            if (is_active) {
                // Second click received! Double click!
                strncpy(rt.last_event, "double_click", sizeof(rt.last_event));
                if (cfg.log_enabled) {
                    EventLog.log(SUBSYS_INPUT, "%s: Double click", cfg.name);
                }
                if (cfg.mqtt_pub) {
                    Mqtt.publishInputEvent(i, "double_click");
                }
                executeAction(cfg.double_action, cfg.target_relay, cfg.target_scene, cfg.name);

                rt.logical_active = true;
                rt.press_start_ms = now;
                rt.long_press_fired = true; // prevent long press on 2nd click
                rt.state = IN_STATE_PRESSED;
                WebServerMgr.broadcastState();
            } else if ((now - rt.state_timer_ms) >= cfg.double_click_ms) {
                // Double click window timed out without 2nd click: It was a Single (Short) Press!
                strncpy(rt.last_event, "short_press", sizeof(rt.last_event));
                if (cfg.log_enabled) {
                    EventLog.log(SUBSYS_INPUT, "%s: Short press", cfg.name);
                }
                if (cfg.mqtt_pub) {
                    Mqtt.publishInputEvent(i, "short_press");
                }

                executeAction(cfg.short_action, cfg.target_relay, cfg.target_scene, cfg.name);

                rt.state = IN_STATE_IDLE;
                WebServerMgr.broadcastState();
            }
            break;
    }
}

void InputSubsystem::update() {
    unsigned long now = millis();
    if (now - last_poll_ms < 10) return;
    last_poll_ms = now;

    uint16_t raw_word = HW.readInputsRaw();

    for (uint8_t i = 0; i < NUM_INPUTS; ++i) {
        if (!configs[i].enabled || configs[i].mode == MODE_DISABLED) {
            runtimes[i].state = IN_STATE_IDLE;
            runtimes[i].logical_active = false;
            continue;
        }

        bool raw_bit = (raw_word >> i) & 1;
        runtimes[i].physical_raw = raw_bit;
        bool is_active = (configs[i].polarity == ACTIVE_LOW) ? (!raw_bit) : (raw_bit);

        processInputFsm(i, is_active);
    }
}

void InputSubsystem::updateConfig(uint8_t index, const InputConfig &cfg) {
    if (index >= NUM_INPUTS) return;
    configs[index] = cfg;
    Storage.saveInput(index, cfg);
    EventLog.log(SUBSYS_INPUT, "%s config updated", cfg.name);
}

void InputSubsystem::updateDefaults(const InputDefaults &defs) {
    defaults = defs;
    Storage.saveInputDefaults(defs);
    EventLog.log(SUBSYS_INPUT, "Global input defaults updated");
}

void InputSubsystem::applyDefaultsToAll() {
    for (uint8_t i = 0; i < NUM_INPUTS; ++i) {
        configs[i].debounce_ms = defaults.debounce_ms;
        configs[i].long_press_ms = defaults.long_press_ms;
        configs[i].double_click_ms = defaults.double_click_ms;
        configs[i].signal_timeout_s = defaults.signal_timeout_s;
        configs[i].timeout_action = defaults.timeout_action;
        Storage.saveInput(i, configs[i]);
    }
    EventLog.log(SUBSYS_INPUT, "Global defaults applied to all inputs");
}

void InputSubsystem::resetInputToDefaults(uint8_t index) {
    if (index >= NUM_INPUTS) return;
    configs[index].debounce_ms = defaults.debounce_ms;
    configs[index].long_press_ms = defaults.long_press_ms;
    configs[index].double_click_ms = defaults.double_click_ms;
    configs[index].signal_timeout_s = defaults.signal_timeout_s;
    configs[index].timeout_action = defaults.timeout_action;
    Storage.saveInput(index, configs[index]);
    EventLog.log(SUBSYS_INPUT, "%s reset to global defaults", configs[index].name);
}
