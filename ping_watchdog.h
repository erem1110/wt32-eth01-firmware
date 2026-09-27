#pragma once

#include <Arduino.h>
#include <WiFiClient.h>
#include "types.h"

class PingWatchdogManager {
public:
    PingWatchdogManager();
    void begin();
    void update();

    const PingWatchdogConfig& getConfig() const { return config; }
    const PingWatchdogRuntime& getRuntime() const { return runtime; }
    void updateConfig(const PingWatchdogConfig &cfg);

    bool triggerTest();

private:
    PingWatchdogConfig config;
    PingWatchdogRuntime runtime;

    bool checkTarget();
    void startPowerCycle();
};

extern PingWatchdogManager PingWatchdog;
