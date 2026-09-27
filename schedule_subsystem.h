#pragma once

#include <Arduino.h>
#include "config.h"
#include "types.h"

class ScheduleSubsystem {
public:
    ScheduleSubsystem();
    void begin();
    void update();

    const ScheduleItem& getSchedule(uint8_t idx) const;
    void updateSchedule(uint8_t idx, const ScheduleItem &item);

private:
    ScheduleItem schedules[NUM_SCHEDULES];
    int8_t last_triggered_min;
    unsigned long last_poll_ms;
    void executeSchedule(const ScheduleItem &item);
};

extern ScheduleSubsystem Schedules;
