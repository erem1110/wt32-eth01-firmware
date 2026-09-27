#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "types.h"

class StorageManager {
public:
    StorageManager();
    void begin();

    void loadSecurity(SecurityConfig &cfg);
    void saveSecurity(const SecurityConfig &cfg);

    void loadNetwork(NetworkConfig &cfg);
    void saveNetwork(const NetworkConfig &cfg);

    void loadMqtt(MqttConfig &cfg);
    void saveMqtt(const MqttConfig &cfg);

    void loadTime(TimeConfig &cfg);
    void saveTime(const TimeConfig &cfg);

    void loadEmergency(EmergencyConfig &cfg, bool &locked);
    void saveEmergency(const EmergencyConfig &cfg);
    void setEmergencyLock(bool locked);

    void loadInputDefaults(InputDefaults &defs);
    void saveInputDefaults(const InputDefaults &defs);

    void loadRelay(uint8_t idx, RelayConfig &cfg, bool &last_state);
    void saveRelay(uint8_t idx, const RelayConfig &cfg);
    void saveRelayLastState(uint8_t idx, bool state);

    void loadInput(uint8_t idx, InputConfig &cfg);
    void saveInput(uint8_t idx, const InputConfig &cfg);

    void loadScene(uint8_t idx, SceneConfig &cfg);
    void saveScene(uint8_t idx, const SceneConfig &cfg);

    void loadSchedule(uint8_t idx, ScheduleItem &item);
    void saveSchedule(uint8_t idx, const ScheduleItem &item);

    void loadWifi(WifiConfig &cfg);
    void saveWifi(const WifiConfig &cfg);

    void loadPingWatchdog(PingWatchdogConfig &cfg);
    void savePingWatchdog(const PingWatchdogConfig &cfg);

    void loadRelayEnergy(uint8_t idx, RelayEnergyStats &stats);
    void saveRelayEnergy(uint8_t idx, const RelayEnergyStats &stats);

    void loadLogicRule(uint8_t idx, LogicRule &rule);
    void saveLogicRule(uint8_t idx, const LogicRule &rule);

    void factoryReset();


private:
    Preferences prefs;
    void initDefaults();
};

extern StorageManager Storage;
