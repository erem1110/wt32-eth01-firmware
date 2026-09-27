#include "ntp_manager.h"
#include "storage.h"
#include "event_log.h"
#include "astro_clock.h"

NtpManager Ntp;

void getFormattedTime(char *buf, size_t len) {
    Ntp.getTimeString(buf, len);
}

NtpManager::NtpManager() : time_synced(false), last_check_ms(0) {}

void NtpManager::begin() {
    Storage.loadTime(config);
    syncTime();
}

void NtpManager::syncTime() {
    configTime(config.gmt_offset_s, config.dst_offset_s, config.ntp_server);
}

void NtpManager::updateConfig(const TimeConfig &cfg) {
    config = cfg;
    Storage.saveTime(config);
    syncTime();
    time_synced = false;
    time_t now = time(nullptr);
    if (now > 100000) {
        Astro.update(now, config.latitude, config.longitude, config.gmt_offset_s);
    }
    EventLog.log(SUBSYS_SYSTEM, "NTP configuration updated (Lat: %.2f, Lon: %.2f)", config.latitude, config.longitude);
}

void NtpManager::getTimeString(char *buf, size_t len) {
    time_t now = time(nullptr);
    struct tm timeinfo;
    if (localtime_r(&now, &timeinfo) && timeinfo.tm_year > (2020 - 1900)) {
        time_synced = true;
        Astro.update(now, config.latitude, config.longitude, config.gmt_offset_s);
        strftime(buf, len, "%Y-%m-%d %H:%M:%S", &timeinfo);
    } else {

        time_synced = false;
        unsigned long s = millis() / 1000UL;
        unsigned int sec = s % 60;
        unsigned int min = (s / 60) % 60;
        unsigned int hr = (s / 3600) % 24;
        unsigned int day = s / 86400;
        snprintf(buf, len, "+%ud %02u:%02u:%02u", day, hr, min, sec);
    }
}

void NtpManager::update() {
    unsigned long now = millis();
    if (now - last_check_ms > 3600000UL) {
        last_check_ms = now;
        syncTime();
    }
}

void NtpManager::setManualTime(time_t epoch, int32_t gmt_offset_s) {
    if (gmt_offset_s != -1) {
        config.gmt_offset_s = gmt_offset_s;
        Storage.saveTime(config);
    }
    struct timeval tv = { .tv_sec = epoch, .tv_usec = 0 };
    settimeofday(&tv, nullptr);
    time_synced = true;
    Astro.update(epoch, config.latitude, config.longitude, config.gmt_offset_s);
    EventLog.log(SUBSYS_SYSTEM, "Clock set manually or from browser (Epoch: %ld)", (long)epoch);
}

void NtpManager::forceSync() {
    syncTime();
    last_check_ms = millis();
    EventLog.log(SUBSYS_SYSTEM, "NTP manual re-sync requested");
}

