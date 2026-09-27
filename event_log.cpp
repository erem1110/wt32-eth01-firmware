#include "event_log.h"
#include <stdio.h>
#include <stdarg.h>

EventLogManager EventLog;

extern void getFormattedTime(char *buf, size_t len);

EventLogManager::EventLogManager() : head(0), total_count(0) {}

void EventLogManager::begin() {
    head = 0;
    total_count = 0;
    log(SUBSYS_SYSTEM, "Firmware initialized");
}

void EventLogManager::log(Subsystem subsys, const char *fmt, ...) {
    LogEntry &entry = buffer[head];
    getFormattedTime(entry.timestamp, sizeof(entry.timestamp));
    entry.subsys = subsys;

    va_list args;
    va_start(args, fmt);
    vsnprintf(entry.message, sizeof(entry.message), fmt, args);
    va_end(args);

    head = (head + 1) % MAX_EVENT_LOGS;
    if (total_count < MAX_EVENT_LOGS) {
        total_count++;
    }

    Serial.printf("[%s][%u] %s\n", entry.timestamp, (uint8_t)entry.subsys, entry.message);
}

void EventLogManager::clear() {
    head = 0;
    total_count = 0;
    log(SUBSYS_SYSTEM, "Event log cleared");
}

uint16_t EventLogManager::count() const {
    return total_count;
}

const LogEntry* EventLogManager::getEntry(uint16_t index) const {
    if (index >= total_count) return nullptr;
    uint16_t start_idx = (head >= total_count) ? (head - total_count) : (MAX_EVENT_LOGS + head - total_count);
    uint16_t actual_idx = (start_idx + index) % MAX_EVENT_LOGS;
    return &buffer[actual_idx];
}

void EventLogManager::getRecentJson(String &output, uint8_t max_items) const {
    output = "[";
    if (total_count == 0) {
        output += "]";
        return;
    }

    uint8_t items = (total_count < max_items) ? total_count : max_items;
    bool first = true;
    for (int i = total_count - 1; i >= total_count - items; --i) {
        const LogEntry *e = getEntry(i);
        if (!e) continue;
        if (!first) output += ",";
        first = false;

        output += "{\"t\":\"";
        output += e->timestamp;
        output += "\",\"s\":";
        output += (uint8_t)e->subsys;
        output += ",\"m\":\"";
        for (const char *p = e->message; *p; ++p) {
            if (*p == '\"') output += "\\\"";
            else if (*p == '\\') output += "\\\\";
            else if (*p == '\n') output += " ";
            else output += *p;
        }
        output += "\"}";
    }
    output += "]";
}

void EventLogManager::getAllJson(String &output, int8_t filter_subsys) const {
    output = "[";
    bool first = true;
    for (int i = (int)total_count - 1; i >= 0; --i) {
        const LogEntry *e = getEntry(i);
        if (!e) continue;
        if (filter_subsys >= 0 && (uint8_t)e->subsys != (uint8_t)filter_subsys) continue;

        if (!first) output += ",";
        first = false;

        output += "{\"t\":\"";
        output += e->timestamp;
        output += "\",\"s\":";
        output += (uint8_t)e->subsys;
        output += ",\"m\":\"";
        for (const char *p = e->message; *p; ++p) {
            if (*p == '\"') output += "\\\"";
            else if (*p == '\\') output += "\\\\";
            else if (*p == '\n') output += " ";
            else output += *p;
        }
        output += "\"}";
    }
    output += "]";
}
