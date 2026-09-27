#pragma once

#include <Arduino.h>
#include "config.h"

struct AutoOffTimerRecord {
    bool active;
    uint32_t target_epoch;
    unsigned long expire_ms;
};

class SafeTimerManager {
public:
    SafeTimerManager();
    void begin();
    void update();

    void scheduleAutoOff(uint8_t relay_idx, uint32_t epoch, uint32_t seconds);
    void cancelAutoOff(uint8_t relay_idx);
    bool isAutoOffActive(uint8_t relay_idx) const;
    uint32_t getRemainingSeconds(uint8_t relay_idx) const;

private:
    AutoOffTimerRecord auto_off_timers[NUM_RELAYS];
};

extern SafeTimerManager TimerMgr;
