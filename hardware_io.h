#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "types.h"

class HardwareIO {
public:
    HardwareIO();
    bool begin();
    void setRelayPhysical(uint8_t index, bool on);
    void writeRelayOutputs();
    uint16_t readInputsRaw();

    bool isOut1Healthy() const { return out1_ok; }
    bool isOut2Healthy() const { return out2_ok; }
    bool isIn1Healthy() const { return in1_ok; }
    bool isIn2Healthy() const { return in2_ok; }
    uint16_t getErrorCount() const { return i2c_error_count; }
    void recoverBus();
    void checkFactoryResetButton();

private:
    uint8_t out1_state;
    uint8_t out2_state;
    uint8_t prev_out1_state;
    uint8_t prev_out2_state;

    bool out1_ok;
    bool out2_ok;
    bool in1_ok;
    bool in2_ok;
    uint16_t i2c_error_count;

    bool writePCF(uint8_t addr, uint8_t val);
    bool readPCF(uint8_t addr, uint8_t &val);
};

extern HardwareIO HW;
