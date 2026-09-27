#include "ping_watchdog.h"
#include "storage.h"
#include "relay_subsystem.h"
#include "network_manager.h"
#include "event_log.h"
#include "web_server.h"

PingWatchdogManager PingWatchdog;

PingWatchdogManager::PingWatchdogManager() {
    memset(&runtime, 0, sizeof(runtime));
    strncpy(runtime.last_result, "Not checked", sizeof(runtime.last_result));
}

void PingWatchdogManager::begin() {
    Storage.loadPingWatchdog(config);
    runtime.last_check_ms = millis();
    runtime.is_checking = false;
    runtime.current_fails = 0;
    runtime.total_reboots = 0;
    runtime.is_power_cycling = false;
    runtime.in_cooldown = false;
    runtime.state_timer_ms = 0;
    strncpy(runtime.last_result, config.enabled ? "Idle" : "Disabled", sizeof(runtime.last_result));
}

void PingWatchdogManager::updateConfig(const PingWatchdogConfig &cfg) {
    config = cfg;
    Storage.savePingWatchdog(config);
    runtime.current_fails = 0;
    runtime.is_power_cycling = false;
    runtime.in_cooldown = false;
    strncpy(runtime.last_result, config.enabled ? "Updated (Active)" : "Disabled", sizeof(runtime.last_result));
    EventLog.log(SUBSYS_WATCHDOG, "Watchdog settings updated (enabled: %s, target: %s:%u)",
                 config.enabled ? "YES" : "NO", config.host, config.port);
    WebServerMgr.broadcastState();
}

bool PingWatchdogManager::checkTarget() {
    if (!Network.isConnected()) {
        strncpy(runtime.last_result, "No network connection", sizeof(runtime.last_result));
        return false;
    }

    WiFiClient client;
    client.setTimeout(2); // 2 second timeout
    unsigned long start_ms = millis();
    bool ok = client.connect(config.host, config.port, 2000);
    unsigned long duration_ms = millis() - start_ms;
    client.stop();

    if (ok) {
        snprintf(runtime.last_result, sizeof(runtime.last_result), "OK (%lu ms)", duration_ms);
        runtime.current_fails = 0;
        return true;
    } else {
        runtime.current_fails++;
        snprintf(runtime.last_result, sizeof(runtime.last_result), "Failed (%u/%u)",
                 runtime.current_fails, config.fail_threshold);
        EventLog.log(SUBSYS_WATCHDOG, "Watchdog ping to %s:%u failed (%u/%u)",
                     config.host, config.port, runtime.current_fails, config.fail_threshold);
        return false;
    }
}

bool PingWatchdogManager::triggerTest() {
    if (strlen(config.host) == 0) return false;
    WiFiClient client;
    client.setTimeout(3);
    unsigned long start_ms = millis();
    bool ok = client.connect(config.host, config.port, 3000);
    unsigned long duration_ms = millis() - start_ms;
    client.stop();

    if (ok) {
        snprintf(runtime.last_result, sizeof(runtime.last_result), "Test OK (%lu ms)", duration_ms);
        EventLog.log(SUBSYS_WATCHDOG, "Manual ping to %s:%u succeeded in %lu ms", config.host, config.port, duration_ms);
    } else {
        snprintf(runtime.last_result, sizeof(runtime.last_result), "Test FAILED");
        EventLog.log(SUBSYS_WATCHDOG, "Manual ping to %s:%u failed", config.host, config.port);
    }
    WebServerMgr.broadcastState();
    return ok;
}

void PingWatchdogManager::startPowerCycle() {
    if (config.target_relay < 1 || config.target_relay > NUM_RELAYS) return;

    runtime.is_power_cycling = true;
    runtime.state_timer_ms = millis();

    uint8_t r_idx = config.target_relay - 1;
    Relays.turnRelayOff(r_idx, SRC_SYSTEM); // Turn OFF router power

    EventLog.log(SUBSYS_WATCHDOG, "Router Watchdog triggered: Relay %u turned OFF for %u s",
                 config.target_relay, config.power_off_duration_s);
    snprintf(runtime.last_result, sizeof(runtime.last_result), "Power cycling (OFF)");
    WebServerMgr.broadcastState();
}

void PingWatchdogManager::update() {
    if (!config.enabled) return;

    unsigned long now = millis();

    // State 1: Power cycling (waiting for power-off period to end)
    if (runtime.is_power_cycling) {
        if (now - runtime.state_timer_ms >= ((unsigned long)config.power_off_duration_s * 1000UL)) {
            runtime.is_power_cycling = false;
            runtime.in_cooldown = true;
            runtime.state_timer_ms = now;
            runtime.total_reboots++;
            runtime.current_fails = 0;

            uint8_t r_idx = config.target_relay - 1;
            Relays.turnRelayOn(r_idx, SRC_SYSTEM); // Turn router back ON!

            EventLog.log(SUBSYS_WATCHDOG, "Router Watchdog: Relay %u turned ON. Cooldown %u s started.",
                         config.target_relay, config.cooldown_s);
            snprintf(runtime.last_result, sizeof(runtime.last_result), "Rebooted (cooling down)");
            WebServerMgr.broadcastState();
        }
        return;
    }

    // State 2: In cooldown after reboot (give router time to boot)
    if (runtime.in_cooldown) {
        if (now - runtime.state_timer_ms >= ((unsigned long)config.cooldown_s * 1000UL)) {
            runtime.in_cooldown = false;
            runtime.last_check_ms = now;
            EventLog.log(SUBSYS_WATCHDOG, "Router Watchdog: Cooldown ended, resuming monitoring.");
            snprintf(runtime.last_result, sizeof(runtime.last_result), "Cooldown finished");
            WebServerMgr.broadcastState();
        }
        return;
    }

    // State 3: Periodic check
    uint32_t interval_ms = ((uint32_t)config.check_interval_s > 5 ? config.check_interval_s : 60) * 1000UL;
    if (now - runtime.last_check_ms >= interval_ms) {
        runtime.last_check_ms = now;
        bool ok = checkTarget();

        if (!ok && runtime.current_fails >= config.fail_threshold) {
            startPowerCycle();
        } else {
            WebServerMgr.broadcastState();
        }
    }
}
