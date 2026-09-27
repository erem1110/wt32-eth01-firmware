#include "schedule_subsystem.h"
#include "storage.h"
#include "event_log.h"
#include "relay_subsystem.h"
#include "scene_subsystem.h"
#include "ntp_manager.h"
#include "astro_clock.h"
#include <time.h>

ScheduleSubsystem Schedules;

ScheduleSubsystem::ScheduleSubsystem() : last_triggered_min(-1), last_poll_ms(0) {
    memset(schedules, 0, sizeof(schedules));
}

void ScheduleSubsystem::begin() {
    for (uint8_t i = 0; i < NUM_SCHEDULES; ++i) {
        Storage.loadSchedule(i, schedules[i]);
    }
}

const ScheduleItem& ScheduleSubsystem::getSchedule(uint8_t idx) const {
    if (idx >= NUM_SCHEDULES) return schedules[0];
    return schedules[idx];
}

void ScheduleSubsystem::updateSchedule(uint8_t idx, const ScheduleItem &item) {
    if (idx >= NUM_SCHEDULES) return;
    schedules[idx] = item;
    Storage.saveSchedule(idx, item);
    EventLog.log(SUBSYS_SYSTEM, "Schedule %u updated", idx + 1);
}

void ScheduleSubsystem::executeSchedule(const ScheduleItem &item) {
    switch (item.action) {
        case ACT_RELAY_ON:
            if (item.target_relay >= 1 && item.target_relay <= NUM_RELAYS) {
                Relays.turnRelayOn(item.target_relay - 1, SRC_TIMER);
            }
            break;
        case ACT_RELAY_OFF:
            if (item.target_relay >= 1 && item.target_relay <= NUM_RELAYS) {
                Relays.turnRelayOff(item.target_relay - 1, SRC_TIMER);
            }
            break;
        case ACT_RELAY_TOGGLE:
            if (item.target_relay >= 1 && item.target_relay <= NUM_RELAYS) {
                Relays.toggleRelay(item.target_relay - 1, SRC_TIMER);
            }
            break;
        case ACT_ACTIVATE_SCENE:
            if (item.target_scene >= 1 && item.target_scene <= NUM_SCENES) {
                Scenes.activateScene(item.target_scene - 1);
            }
            break;
        case ACT_ALL_RELAYS_OFF:
            Relays.allRelaysOff(SRC_TIMER);
            break;
        case ACT_ALL_RELAYS_ON:
            Relays.allRelaysOn(SRC_TIMER);
            break;
        default:
            break;
    }
}

void ScheduleSubsystem::update() {
    unsigned long now_ms = millis();
    if (now_ms - last_poll_ms < 1000) return;
    last_poll_ms = now_ms;

    if (!Ntp.isSynced()) return;

    time_t now = time(nullptr);
    struct tm timeinfo;
    if (!localtime_r(&now, &timeinfo)) return;

    int current_min_of_day = timeinfo.tm_hour * 60 + timeinfo.tm_min;
    if (current_min_of_day == last_triggered_min) return;

    for (uint8_t i = 0; i < NUM_SCHEDULES; ++i) {
        if (!schedules[i].enabled) continue;
        if ((schedules[i].days_mask & (1 << timeinfo.tm_wday)) == 0) continue;

        int target_min_of_day = 0;
        if (schedules[i].trigger_type == TRIGGER_SUNRISE) {
            target_min_of_day = (int)Astro.getSunriseMinute() + schedules[i].offset_minutes;
        } else if (schedules[i].trigger_type == TRIGGER_SUNSET) {
            target_min_of_day = (int)Astro.getSunsetMinute() + schedules[i].offset_minutes;
        } else {
            // TRIGGER_CLOCK
            target_min_of_day = schedules[i].hour * 60 + schedules[i].minute;
        }

        while (target_min_of_day < 0) target_min_of_day += 1440;
        while (target_min_of_day >= 1440) target_min_of_day -= 1440;

        if (current_min_of_day == target_min_of_day) {
            const char *trig_name = (schedules[i].trigger_type == TRIGGER_SUNRISE) ? "Sunrise" :
                                    (schedules[i].trigger_type == TRIGGER_SUNSET)  ? "Sunset" : "Clock";
            EventLog.log(SUBSYS_SYSTEM, "Schedule '%s' triggered (%s @ %02d:%02d)",
                         schedules[i].name, trig_name, timeinfo.tm_hour, timeinfo.tm_min);
            executeSchedule(schedules[i]);
        }
    }

    last_triggered_min = current_min_of_day;
}

