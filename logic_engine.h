#pragma once

#include <Arduino.h>
#include "types.h"
#include "config.h"

class LogicEngine {
public:
    LogicEngine();
    void begin();
    void update();

    void evaluate(CommandSource trigger_source);

    const LogicRule& getRule(uint8_t idx) const;
    void updateRule(uint8_t idx, const LogicRule &rule);

private:
    LogicRule rules[NUM_LOGIC_RULES];
    LogicRuleRuntime runtimes[NUM_LOGIC_RULES];
    unsigned long last_eval_ms;

    bool evaluateCondition(LogicConditionType type, uint8_t target);
    void executeRuleAction(InputAction action, uint8_t target, const char *rule_name);
};

extern LogicEngine Logic;
