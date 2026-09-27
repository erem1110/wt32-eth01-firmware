#pragma once

#include <Arduino.h>

#define FIRMWARE_NAME       "WT32-ETH01 Relay Controller"
#define FIRMWARE_VERSION    "1.0.0"

#define ETH_PHY_TYPE        ETH_PHY_LAN8720
#define ETH_PHY_ADDR        1
#define ETH_PHY_MDC         23
#define ETH_PHY_MDIO        18
#define ETH_PHY_POWER       16
#define ETH_CLK_MODE        ETH_CLOCK_GPIO0_IN

#define I2C_SDA_PIN         14
#define I2C_SCL_PIN         15
#define I2C_CLOCK_SPEED     100000

#define PCF_OUT1_ADDR       0x21
#define PCF_OUT2_ADDR       0x20
#define PCF_IN1_ADDR        0x22
#define PCF_IN2_ADDR        0x23

#define FACTORY_RESET_PIN       32    // WT32-ETH01 "CFG" pin (bridge to GND for 8s to factory reset)
#define FACTORY_RESET_HOLD_MS   8000  // 8 seconds hold time for factory reset

#define NUM_RELAYS          16
#define NUM_INPUTS          16
#define NUM_SCENES          8
#define NUM_INTERLOCK_GROUPS 8
#define NUM_SCHEDULES       8
#define NUM_LOGIC_RULES     8

#define DEFAULT_LATITUDE    40.18f
#define DEFAULT_LONGITUDE   44.51f


#define HTTP_PORT           80
#define WS_PORT             81

#define MAX_EVENT_LOGS      100
#define DEFAULT_DEBOUNCE_MS 50
#define DEFAULT_LONG_PRESS  1500
#define DEFAULT_DBL_CLICK   350
#define DEFAULT_SIG_TIMEOUT 0
#define DEFAULT_SESSION_EXP 900
#define MAX_FAILED_LOGINS   5
#define LOCKOUT_DURATION_S  300
