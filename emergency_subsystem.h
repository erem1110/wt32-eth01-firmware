#pragma once

#include <Arduino.h>
#include "config.h"
#include "types.h"

class EmergencySubsystem {
public:
    EmergencySubsystem();
    void begin();
    void update();

    bool isEmergencyActive() const { return runtime.active; }
    bool isRelaysLocked() const { return runtime.locked; }
    const EmergencyConfig& getConfig() const { return config; }
    const EmergencyRuntime& getRuntime() const { return runtime; }

    void triggerEmergency(const char *source);
    bool resetEmergency(bool confirmed = true);
    void updateConfig(const EmergencyConfig &cfg);

private:
    EmergencyConfig config;
    EmergencyRuntime runtime;
    void executeEmergencyActions();
};

extern EmergencySubsystem Emergency;
