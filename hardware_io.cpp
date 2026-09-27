#include "hardware_io.h"
#include "event_log.h"
#include "storage.h"

HardwareIO HW;

static const RelayHwPin RELAY_HW_MAP[NUM_RELAYS] = {
    { PCF_OUT2_ADDR, 7 }, // R1  -> out2 P7
    { PCF_OUT1_ADDR, 0 }, // R2  -> out1 P0
    { PCF_OUT2_ADDR, 6 }, // R3  -> out2 P6
    { PCF_OUT1_ADDR, 1 }, // R4  -> out1 P1
    { PCF_OUT2_ADDR, 5 }, // R5  -> out2 P5
    { PCF_OUT1_ADDR, 2 }, // R6  -> out1 P2
    { PCF_OUT2_ADDR, 4 }, // R7  -> out2 P4
    { PCF_OUT1_ADDR, 3 }, // R8  -> out1 P3
    { PCF_OUT2_ADDR, 3 }, // R9  -> out2 P3
    { PCF_OUT1_ADDR, 4 }, // R10 -> out1 P4
    { PCF_OUT2_ADDR, 2 }, // R11 -> out2 P2
    { PCF_OUT1_ADDR, 5 }, // R12 -> out1 P5
    { PCF_OUT2_ADDR, 1 }, // R13 -> out2 P1
    { PCF_OUT1_ADDR, 6 }, // R14 -> out1 P6
    { PCF_OUT2_ADDR, 0 }, // R15 -> out2 P0
    { PCF_OUT1_ADDR, 7 }  // R16 -> out1 P7
};

HardwareIO::HardwareIO()
    : out1_state(0xFF), out2_state(0xFF),
      prev_out1_state(0x00), prev_out2_state(0x00),
      out1_ok(false), out2_ok(false), in1_ok(false), in2_ok(false),
      i2c_error_count(0) {}

void HardwareIO::recoverBus() {
    EventLog.log(SUBSYS_SYSTEM, "I2C bus recovery initiated");
    Wire.end();
    pinMode(I2C_SDA_PIN, INPUT_PULLUP);
    pinMode(I2C_SCL_PIN, OUTPUT);
    digitalWrite(I2C_SCL_PIN, HIGH);
    delayMicroseconds(10);

    for (int i = 0; i < 9; i++) {
        digitalWrite(I2C_SCL_PIN, LOW);
        delayMicroseconds(10);
        digitalWrite(I2C_SCL_PIN, HIGH);
        delayMicroseconds(10);
    }

    pinMode(I2C_SDA_PIN, OUTPUT);
    digitalWrite(I2C_SDA_PIN, LOW);
    delayMicroseconds(10);
    digitalWrite(I2C_SCL_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(I2C_SDA_PIN, HIGH);
    delayMicroseconds(10);

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_CLOCK_SPEED);

    Wire.beginTransmission(PCF_OUT1_ADDR);
    Wire.write(out1_state);
    out1_ok = (Wire.endTransmission() == 0);

    Wire.beginTransmission(PCF_OUT2_ADDR);
    Wire.write(out2_state);
    out2_ok = (Wire.endTransmission() == 0);

    i2c_error_count = 0;
    EventLog.log(SUBSYS_SYSTEM, "I2C bus recovery complete");
}

bool HardwareIO::writePCF(uint8_t addr, uint8_t val) {
    Wire.beginTransmission(addr);
    Wire.write(val);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
        i2c_error_count = 0;
        return true;
    }
    i2c_error_count++;
    if (i2c_error_count >= 3) {
        recoverBus();
    }
    return false;
}

bool HardwareIO::readPCF(uint8_t addr, uint8_t &val) {
    uint8_t count = Wire.requestFrom((uint8_t)addr, (uint8_t)1);
    if (count == 1 && Wire.available()) {
        val = Wire.read();
        i2c_error_count = 0;
        return true;
    }
    i2c_error_count++;
    if (i2c_error_count >= 3) {
        recoverBus();
    }
    return false;
}

bool HardwareIO::begin() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_CLOCK_SPEED);

    out1_ok = writePCF(PCF_OUT1_ADDR, 0xFF);
    out2_ok = writePCF(PCF_OUT2_ADDR, 0xFF);

    in1_ok = writePCF(PCF_IN1_ADDR, 0xFF);
    in2_ok = writePCF(PCF_IN2_ADDR, 0xFF);

    out1_state = 0xFF;
    out2_state = 0xFF;
    prev_out1_state = 0xFF;
    prev_out2_state = 0xFF;

    pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);

    if (!out1_ok) EventLog.log(SUBSYS_SYSTEM, "I2C PCF_OUT1 @ 0x21 failed");
    if (!out2_ok) EventLog.log(SUBSYS_SYSTEM, "I2C PCF_OUT2 @ 0x20 failed");
    if (!in1_ok)  EventLog.log(SUBSYS_SYSTEM, "I2C PCF_IN1 @ 0x22 failed");
    if (!in2_ok)  EventLog.log(SUBSYS_SYSTEM, "I2C PCF_IN2 @ 0x23 failed");

    // Check if CFG pin (GPIO 32) is held at boot for 5 seconds -> Power-on Factory Reset
    if (digitalRead(FACTORY_RESET_PIN) == LOW) {
        unsigned long boot_press_start = millis();
        while (digitalRead(FACTORY_RESET_PIN) == LOW && (millis() - boot_press_start < 5000)) {
            delay(50);
        }
        if (digitalRead(FACTORY_RESET_PIN) == LOW) {
            Serial.println(F("[SYSTEM] Power-on Factory Reset triggered via CFG pin (GPIO 32)!"));
            Storage.factoryReset();
            delay(500);
            ESP.restart();
        }
    }

    return (out1_ok && out2_ok && in1_ok && in2_ok);
}

void HardwareIO::setRelayPhysical(uint8_t index, bool on) {
    if (index >= NUM_RELAYS) return;
    const RelayHwPin &pin = RELAY_HW_MAP[index];

    if (pin.i2c_addr == PCF_OUT1_ADDR) {
        if (on) {
            out1_state &= ~(1 << pin.pin_bit);
        } else {
            out1_state |= (1 << pin.pin_bit);
        }
    } else if (pin.i2c_addr == PCF_OUT2_ADDR) {
        if (on) {
            out2_state &= ~(1 << pin.pin_bit);
        } else {
            out2_state |= (1 << pin.pin_bit);
        }
    }
}

void HardwareIO::writeRelayOutputs() {
    if (out1_state != prev_out1_state) {
        if (writePCF(PCF_OUT1_ADDR, out1_state)) {
            prev_out1_state = out1_state;
            out1_ok = true;
        } else {
            out1_ok = false;
        }
    }

    if (out2_state != prev_out2_state) {
        if (writePCF(PCF_OUT2_ADDR, out2_state)) {
            prev_out2_state = out2_state;
            out2_ok = true;
        } else {
            out2_ok = false;
        }
    }
}

uint16_t HardwareIO::readInputsRaw() {
    static uint8_t last_val1 = 0xFF;
    static uint8_t last_val2 = 0xFF;
    uint8_t val1 = last_val1;
    uint8_t val2 = last_val2;

    if (readPCF(PCF_IN1_ADDR, val1)) {
        last_val1 = val1;
        in1_ok = true;
    } else {
        in1_ok = false;
        val1 = last_val1; // Glitch filter: retain last valid reading on communication error
    }

    if (readPCF(PCF_IN2_ADDR, val2)) {
        last_val2 = val2;
        in2_ok = true;
    } else {
        in2_ok = false;
        val2 = last_val2; // Glitch filter: retain last valid reading on communication error
    }

    return ((uint16_t)val2 << 8) | val1;
}

void HardwareIO::checkFactoryResetButton() {
    static unsigned long press_start_ms = 0;
    static bool reset_done = false;

    if (reset_done) return;

    if (digitalRead(FACTORY_RESET_PIN) == LOW) {
        if (press_start_ms == 0) {
            press_start_ms = millis();
            EventLog.log(SUBSYS_SYSTEM, "CFG pin (GPIO 32) active. Hold 8s for Factory Reset...");
            Serial.println(F("[SYSTEM] CFG pin (GPIO 32) active. Hold 8s for Factory Reset..."));
        } else if (millis() - press_start_ms >= FACTORY_RESET_HOLD_MS) {
            reset_done = true;
            EventLog.log(SUBSYS_SYSTEM, "HARDWARE FACTORY RESET triggered via CFG pin! Resetting device...");
            Serial.println(F("[SYSTEM] HARDWARE FACTORY RESET triggered via CFG pin (GPIO 32)!"));
            delay(200);
            Storage.factoryReset();
            delay(500);
            ESP.restart();
        }
    } else {
        if (press_start_ms != 0) {
            unsigned long held_ms = millis() - press_start_ms;
            press_start_ms = 0;
            if (held_ms >= 500) {
                EventLog.log(SUBSYS_SYSTEM, "CFG pin released after %lu ms (Factory Reset cancelled)", held_ms);
            }
        }
    }
}
