#pragma once

#include <Arduino.h>
#include "config.h"
#include "types.h"

class EventLogManager {
public:
    EventLogManager();
    void begin();
    void log(Subsystem subsys, const char *fmt, ...);
    void clear();
    uint16_t count() const;
    const LogEntry* getEntry(uint16_t index) const;
    void getRecentJson(String &output, uint8_t max_items = 10) const;
    void getAllJson(String &output, int8_t filter_subsys = -1) const;

private:
    LogEntry buffer[MAX_EVENT_LOGS];
    uint16_t head;
    uint16_t total_count;
};

extern EventLogManager EventLog;
