#include "timer_manager.h"

SafeTimerManager TimerMgr;

extern void onAutoOffExpired(uint8_t relay_idx, uint32_t epoch);

SafeTimerManager::SafeTimerManager() {
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        auto_off_timers[i].active = false;
        auto_off_timers[i].target_epoch = 0;
        auto_off_timers[i].expire_ms = 0;
    }
}

void SafeTimerManager::begin() {}

void SafeTimerManager::scheduleAutoOff(uint8_t relay_idx, uint32_t epoch, uint32_t seconds) {
    if (relay_idx >= NUM_RELAYS || seconds == 0) return;
    auto_off_timers[relay_idx].active = true;
    auto_off_timers[relay_idx].target_epoch = epoch;
    auto_off_timers[relay_idx].expire_ms = millis() + (unsigned long)seconds * 1000UL;
}

void SafeTimerManager::cancelAutoOff(uint8_t relay_idx) {
    if (relay_idx >= NUM_RELAYS) return;
    auto_off_timers[relay_idx].active = false;
}

bool SafeTimerManager::isAutoOffActive(uint8_t relay_idx) const {
    if (relay_idx >= NUM_RELAYS) return false;
    return auto_off_timers[relay_idx].active;
}

uint32_t SafeTimerManager::getRemainingSeconds(uint8_t relay_idx) const {
    if (relay_idx >= NUM_RELAYS || !auto_off_timers[relay_idx].active) return 0;
    unsigned long now = millis();
    if (now >= auto_off_timers[relay_idx].expire_ms) return 0;
    return (uint32_t)((auto_off_timers[relay_idx].expire_ms - now + 999UL) / 1000UL);
}

void SafeTimerManager::update() {
    unsigned long now = millis();
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        if (auto_off_timers[i].active && (long)(now - auto_off_timers[i].expire_ms) >= 0) {
            auto_off_timers[i].active = false;
            onAutoOffExpired(i, auto_off_timers[i].target_epoch);
        }
    }
}
