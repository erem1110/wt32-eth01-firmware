#include <Arduino.h>
#include "config.h"
#include "types.h"
#include "storage.h"
#include "event_log.h"
#include "hardware_io.h"
#include "timer_manager.h"
#include "emergency_subsystem.h"
#include "relay_subsystem.h"
#include "input_subsystem.h"
#include "scene_subsystem.h"
#include "network_manager.h"
#include "ntp_manager.h"
#include "mqtt_manager.h"
#include "auth_manager.h"
#include <esp_task_wdt.h>
#include "schedule_subsystem.h"
#include "ping_watchdog.h"
#include "logic_engine.h"
#include "web_server.h"

void setup() {
    Serial.begin(115200);
    delay(200);

    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F(" WT32-ETH01 16-Relay Standalone System "));
    Serial.println(F("========================================"));

    esp_task_wdt_init(15, true);
    esp_task_wdt_add(NULL);

    Storage.begin();
    EventLog.begin();
    Auth.begin();
    HW.begin();
    Emergency.begin();
    Relays.begin();
    Inputs.begin();
    Scenes.begin();
    Schedules.begin();
    TimerMgr.begin();
    Network.begin();
    Ntp.begin();
    Mqtt.begin();
    PingWatchdog.begin();
    Logic.begin();
    WebServerMgr.begin();

    EventLog.log(SUBSYS_SYSTEM, "All subsystems running (WDT active)");
}

void loop() {
    esp_task_wdt_reset();
    HW.checkFactoryResetButton();
    Network.update();
    PingWatchdog.update();
    Inputs.update();
    Relays.update();
    Logic.update();
    esp_task_wdt_reset();
    Schedules.update();
    TimerMgr.update();
    Emergency.update();
    Mqtt.update();
    Ntp.update();
    Auth.update();
    WebServerMgr.update();
    esp_task_wdt_reset();
}

