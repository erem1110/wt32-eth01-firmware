#include "logic_engine.h"
#include "storage.h"
#include "event_log.h"
#include "relay_subsystem.h"
#include "input_subsystem.h"
#include "scene_subsystem.h"
#include "astro_clock.h"

LogicEngine Logic;

LogicEngine::LogicEngine() : last_eval_ms(0) {
    for (uint8_t i = 0; i < NUM_LOGIC_RULES; ++i) {
        memset(&rules[i], 0, sizeof(LogicRule));
        runtimes[i].last_eval_result = false;
    }
}

void LogicEngine::begin() {
    for (uint8_t i = 0; i < NUM_LOGIC_RULES; ++i) {
        Storage.loadLogicRule(i, rules[i]);
        runtimes[i].last_eval_result = false;
    }
}

const LogicRule& LogicEngine::getRule(uint8_t idx) const {
    if (idx >= NUM_LOGIC_RULES) return rules[0];
    return rules[idx];
}

void LogicEngine::updateRule(uint8_t idx, const LogicRule &rule) {
    if (idx >= NUM_LOGIC_RULES) return;
    rules[idx] = rule;
    runtimes[idx].last_eval_result = false; // Reset edge trigger on edit
    Storage.saveLogicRule(idx, rule);
    EventLog.log(SUBSYS_LOGIC, "Rule %u ('%s') updated", idx + 1, rule.name);
}

bool LogicEngine::evaluateCondition(LogicConditionType type, uint8_t target) {
    switch (type) {
        case COND_INPUT_ACTIVE:
            if (target >= 1 && target <= NUM_INPUTS) {
                return Inputs.getRuntime(target - 1).logical_active;
            }
            return false;
        case COND_INPUT_INACTIVE:
            if (target >= 1 && target <= NUM_INPUTS) {
                return !Inputs.getRuntime(target - 1).logical_active;
            }
            return false;
        case COND_RELAY_ON:
            if (target >= 1 && target <= NUM_RELAYS) {
                return Relays.getRelayState(target - 1);
            }
            return false;
        case COND_RELAY_OFF:
            if (target >= 1 && target <= NUM_RELAYS) {
                return !Relays.getRelayState(target - 1);
            }
            return false;
        case COND_IS_NIGHT:
            return Astro.isNight();
        case COND_IS_DAY:
            return !Astro.isNight();
        case COND_NONE:
        default:
            return true;
    }
}

void LogicEngine::executeRuleAction(InputAction action, uint8_t target, const char *rule_name) {
    EventLog.log(SUBSYS_LOGIC, "Logic Rule '%s' fired action %u (target %u)", rule_name, (uint8_t)action, target);
    switch (action) {
        case ACT_RELAY_ON:
            if (target >= 1 && target <= NUM_RELAYS) {
                Relays.turnRelayOn(target - 1, SRC_SYSTEM);
            }
            break;
        case ACT_RELAY_OFF:
            if (target >= 1 && target <= NUM_RELAYS) {
                Relays.turnRelayOff(target - 1, SRC_SYSTEM);
            }
            break;
        case ACT_RELAY_TOGGLE:
            if (target >= 1 && target <= NUM_RELAYS) {
                Relays.toggleRelay(target - 1, SRC_SYSTEM);
            }
            break;
        case ACT_ACTIVATE_SCENE:
            if (target >= 1 && target <= NUM_SCENES) {
                Scenes.activateScene(target - 1);
            }
            break;
        case ACT_ALL_RELAYS_OFF:
            Relays.allRelaysOff(SRC_SYSTEM);
            break;
        case ACT_ALL_RELAYS_ON:
            Relays.allRelaysOn(SRC_SYSTEM);
            break;
        default:
            break;
    }
}

void LogicEngine::evaluate(CommandSource trigger_source) {
    for (uint8_t i = 0; i < NUM_LOGIC_RULES; ++i) {
        if (!rules[i].enabled) {
            runtimes[i].last_eval_result = false;
            continue;
        }

        bool c1 = evaluateCondition(rules[i].cond1_type, rules[i].cond1_target);
        bool final_result = c1;

        if (rules[i].logic_op == LOGIC_OP_AND) {
            bool c2 = evaluateCondition(rules[i].cond2_type, rules[i].cond2_target);
            final_result = c1 && c2;
        } else if (rules[i].logic_op == LOGIC_OP_OR) {
            bool c2 = evaluateCondition(rules[i].cond2_type, rules[i].cond2_target);
            final_result = c1 || c2;
        }

        // Edge detection: fire when transitioning from FALSE to TRUE
        if (final_result && !runtimes[i].last_eval_result) {
            executeRuleAction(rules[i].action, rules[i].action_target, rules[i].name);
        }

        runtimes[i].last_eval_result = final_result;
    }
}

void LogicEngine::update() {
    unsigned long now = millis();
    if (now - last_eval_ms < 500) return; // Evaluate every 500ms
    last_eval_ms = now;

    evaluate(SRC_SYSTEM);
}
