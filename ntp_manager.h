#pragma once

#include <Arduino.h>
#include <time.h>
#include "types.h"

class NtpManager {
public:
    NtpManager();
    void begin();
    void update();

    void updateConfig(const TimeConfig &cfg);
    const TimeConfig& getConfig() const { return config; }
    bool isSynced() const { return time_synced; }
    void getTimeString(char *buf, size_t len);
    void setManualTime(time_t epoch, int32_t gmt_offset_s = -1);
    void forceSync();

private:
    TimeConfig config;
    bool time_synced;
    unsigned long last_check_ms;
    void syncTime();
};

extern NtpManager Ntp;
void getFormattedTime(char *buf, size_t len);
