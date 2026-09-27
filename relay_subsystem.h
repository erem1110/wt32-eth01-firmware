#pragma once

#include <Arduino.h>
#include "config.h"
#include "types.h"

class RelaySubsystem {
public:
    RelaySubsystem();
    void begin();
    void update();

    bool turnRelayOn(uint8_t index, CommandSource source);
    bool turnRelayOff(uint8_t index, CommandSource source);
    bool toggleRelay(uint8_t index, CommandSource source);
    void allRelaysOff(CommandSource source);
    void allRelaysOn(CommandSource source);

    void forceEmergencyShutoff(uint16_t target_mask);
    void handleAutoOffExpired(uint8_t index, uint32_t target_epoch);

    const RelayConfig& getConfig(uint8_t index) const { return configs[index]; }
    const RelayRuntime& getRuntime(uint8_t index) const { return runtimes[index]; }
    void updateConfig(uint8_t index, const RelayConfig &cfg);

    bool getRelayState(uint8_t index) const {
        return (index < NUM_RELAYS) ? runtimes[index].state : false;
    }
    const RelayEnergyStats& getEnergyStats(uint8_t index) const { return runtimes[index].energy; }
    void resetEnergyStats(uint8_t index);
    float getRelayKWh(uint8_t index) const;
    uint32_t getRelayTotalOnSeconds(uint8_t index) const;
    float getTotalSystemPowerW() const;
    float getTotalSystemKWh() const;

private:
    RelayConfig configs[NUM_RELAYS];
    RelayRuntime runtimes[NUM_RELAYS];
    unsigned long last_energy_save_ms;

    void saveAllEnergyStats();
    void applyPhysicalOn(uint8_t index, CommandSource source);
    void applyPhysicalOff(uint8_t index, CommandSource source);
};


extern RelaySubsystem Relays;
