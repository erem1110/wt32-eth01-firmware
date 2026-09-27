#pragma once

#include <Arduino.h>
#include "config.h"
#include "types.h"

class InputSubsystem {
public:
    InputSubsystem();
    void begin();
    void update();

    const InputConfig& getConfig(uint8_t index) const { return configs[index]; }
    const InputRuntime& getRuntime(uint8_t index) const { return runtimes[index]; }
    const InputDefaults& getDefaults() const { return defaults; }

    void updateConfig(uint8_t index, const InputConfig &cfg);
    void updateDefaults(const InputDefaults &defs);
    void applyDefaultsToAll();
    void resetInputToDefaults(uint8_t index);

    bool isInputActive(uint8_t index) const {
        return (index < NUM_INPUTS) ? runtimes[index].logical_active : false;
    }

private:
    InputConfig configs[NUM_INPUTS];
    InputRuntime runtimes[NUM_INPUTS];
    InputDefaults defaults;
    unsigned long last_poll_ms;

    void processInputFsm(uint8_t i, bool is_active);
    void executeAction(InputAction act, uint8_t target_relay, uint8_t target_scene, const char *source_name);
};

extern InputSubsystem Inputs;
