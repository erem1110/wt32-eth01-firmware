#pragma once

#include <Arduino.h>
#include "config.h"

enum RelayRestoreMode : uint8_t {
    RESTORE_ALWAYS_OFF = 0,
    RESTORE_PREVIOUS   = 1,
    RESTORE_ALWAYS_ON  = 2
};

enum RelayPendingState : uint8_t {
    RELAY_IDLE        = 0,
    RELAY_PENDING_ON  = 1,
    RELAY_PENDING_OFF = 2
};

enum CommandSource : uint8_t {
    SRC_WEB       = 0,
    SRC_MQTT      = 1,
    SRC_INPUT     = 2,
    SRC_SCENE     = 3,
    SRC_TIMER     = 4,
    SRC_EMERGENCY = 5,
    SRC_SYSTEM    = 6
};

enum InputPolarity : uint8_t {
    ACTIVE_LOW  = 0,
    ACTIVE_HIGH = 1
};

enum InputMode : uint8_t {
    MODE_TOGGLE       = 0,
    MODE_MOMENTARY    = 1,
    MODE_STATE        = 2,
    MODE_SCENE        = 3,
    MODE_DISABLED     = 4,
    MODE_EMERGENCY    = 5,
    MODE_MULTI_ACTION = 6
};

enum InputAction : uint8_t {
    ACT_NONE               = 0,
    ACT_RELAY_ON           = 1,
    ACT_RELAY_OFF          = 2,
    ACT_RELAY_TOGGLE       = 3,
    ACT_ACTIVATE_SCENE     = 4,
    ACT_ALL_RELAYS_OFF     = 5,
    ACT_ALL_RELAYS_ON      = 6,
    ACT_EMERGENCY_ACTIVATE = 7
};

enum StartupBehavior : uint8_t {
    STARTUP_IGNORE      = 0,
    STARTUP_PROCESS     = 1,
    STARTUP_WAIT_CHANGE = 2
};

enum SignalTimeoutAction : uint8_t {
    TIMEOUT_WARN_ONLY    = 0,
    TIMEOUT_AUTO_RELEASE = 1,
    TIMEOUT_EMERGENCY    = 2
};

enum Subsystem : uint8_t {
    SUBSYS_SYSTEM    = 0,
    SUBSYS_RELAY     = 1,
    SUBSYS_INPUT     = 2,
    SUBSYS_EMERGENCY = 3,
    SUBSYS_NETWORK   = 4,
    SUBSYS_MQTT      = 5,
    SUBSYS_SECURITY  = 6,
    SUBSYS_WATCHDOG  = 7,
    SUBSYS_LOGIC     = 8
};

enum DeviceIcon : uint8_t {
    ICON_LIGHT   = 0,
    ICON_SOCKET  = 1,
    ICON_GATE    = 2,
    ICON_BOILER  = 3,
    ICON_PUMP    = 4,
    ICON_FAN_AC  = 5,
    ICON_GENERAL = 6
};

enum SceneAction : uint8_t {
    SCENE_RELAY_IGNORE = 0,
    SCENE_RELAY_OFF    = 1,
    SCENE_RELAY_ON     = 2
};

struct RelayHwPin {
    uint8_t i2c_addr;
    uint8_t pin_bit;
};

struct RelayConfig {
    char name[24];
    bool enabled;
    RelayRestoreMode restore_mode;
    uint32_t auto_off_sec;
    uint32_t on_delay_ms;
    uint32_t off_delay_ms;
    uint8_t interlock_group;
    bool manual_control_enabled;
    bool mqtt_control_enabled;
    bool input_control_enabled;
    bool event_logging_enabled;
    uint16_t rated_power_w;
    uint8_t device_icon;
    char room[20];
};

struct RelayEnergyStats {
    uint32_t total_cycles;
    uint32_t total_on_seconds;
};

struct RelayRuntime {
    bool state;
    bool target_state;
    bool last_saved_state;
    uint32_t timer_epoch;
    unsigned long auto_off_expire_ms;
    RelayPendingState pending_state;
    unsigned long delay_expire_ms;
    CommandSource pending_source;
    unsigned long current_on_start_ms;
    RelayEnergyStats energy;
};


struct InputConfig {
    char name[24];
    bool enabled;
    InputPolarity polarity;
    uint16_t debounce_ms;
    uint16_t long_press_ms;
    uint16_t double_click_ms;
    uint16_t signal_timeout_s;
    SignalTimeoutAction timeout_action;
    InputMode mode;
    uint8_t target_relay;
    uint8_t target_scene;
    InputAction short_action;
    InputAction long_action;
    InputAction double_action;
    StartupBehavior startup;
    bool is_emergency;
    uint8_t emergency_relay;
    bool dash_visible;
    bool mqtt_pub;
    bool log_enabled;
};

struct InputDefaults {
    uint16_t debounce_ms;
    uint16_t long_press_ms;
    uint16_t double_click_ms;
    uint16_t signal_timeout_s;
    SignalTimeoutAction timeout_action;
};

enum InputFsmState : uint8_t {
    IN_STATE_IDLE             = 0,
    IN_STATE_DEBOUNCE_PRESS   = 1,
    IN_STATE_PRESSED          = 2,
    IN_STATE_DEBOUNCE_RELEASE = 3,
    IN_STATE_WAIT_DOUBLE      = 4
};

struct InputRuntime {
    InputFsmState state;
    bool physical_raw;
    bool logical_active;
    bool prev_logical_active;
    bool long_press_fired;
    bool timeout_warned;
    bool startup_locked;
    unsigned long state_timer_ms;
    unsigned long press_start_ms;
    char last_event[16];
};

struct EmergencyConfig {
    uint16_t trigger_inputs_mask;
    bool turn_off_all;
    uint16_t target_relays_mask;
    bool lock_relays;
    bool manual_reset_only;
    uint32_t auto_reset_seconds;
    bool latch_mode;
};

struct EmergencyRuntime {
    bool active;
    bool locked;
    unsigned long activated_ms;
    char trigger_source[32];
};

struct SceneConfig {
    char name[24];
    SceneAction actions[NUM_RELAYS];
};

struct NetworkConfig {
    bool dhcp;
    char ip[16];
    char gateway[16];
    char subnet[16];
    char dns[16];
    char hostname[32];
};

struct MqttConfig {
    bool enabled;
    bool use_ssl;
    uint8_t ssl_mode;
    char host[64];
    uint16_t port;
    char user[32];
    char pass[64];
    char client_id[32];
    char base_topic[48];
    bool ha_discovery;
};

struct SecurityConfig {
    bool is_configured;
    char username[32];
    char salt[33];
    char pass_hash[65];
    uint32_t session_timeout_s;
    uint8_t lockout_attempts;
    uint32_t lockout_time_s;
};

struct TimeConfig {
    char ntp_server[64];
    int32_t gmt_offset_s;
    int32_t dst_offset_s;
    float latitude;
    float longitude;
};

struct LogEntry {
    char timestamp[24];
    Subsystem subsys;
    char message[80];
};

enum ScheduleTriggerType : uint8_t {
    TRIGGER_CLOCK   = 0,
    TRIGGER_SUNRISE = 1,
    TRIGGER_SUNSET  = 2
};

struct ScheduleItem {
    char name[20];
    bool enabled;
    uint8_t days_mask;
    uint8_t hour;
    uint8_t minute;
    InputAction action;
    uint8_t target_relay;
    uint8_t target_scene;
    ScheduleTriggerType trigger_type;
    int8_t offset_minutes;
};


struct WifiConfig {
    bool sta_enabled;
    char sta_ssid[33];
    char sta_pass[65];
    bool ap_fallback_enabled;
    char ap_ssid[33];
    char ap_pass[33];
};

struct WifiRuntime {
    bool sta_connected;
    char sta_ip[16];
    int8_t sta_rssi;
    bool ap_active;
    char ap_ip[16];
    uint8_t ap_clients;
};

struct PingWatchdogConfig {
    bool enabled;
    char host[64];
    uint16_t port;
    uint16_t check_interval_s;
    uint8_t fail_threshold;
    uint8_t target_relay;
    uint16_t power_off_duration_s;
    uint16_t cooldown_s;
};

struct PingWatchdogRuntime {
    bool is_checking;
    uint8_t current_fails;
    uint32_t total_reboots;
    unsigned long last_check_ms;
    bool is_power_cycling;
    bool in_cooldown;
    unsigned long state_timer_ms;
    char last_result[32];
};

enum LogicConditionType : uint8_t {
    COND_NONE           = 0,
    COND_INPUT_ACTIVE   = 1,
    COND_INPUT_INACTIVE = 2,
    COND_RELAY_ON       = 3,
    COND_RELAY_OFF      = 4,
    COND_IS_NIGHT       = 5,
    COND_IS_DAY         = 6
};

enum LogicOperator : uint8_t {
    LOGIC_OP_NONE = 0,
    LOGIC_OP_AND  = 1,
    LOGIC_OP_OR   = 2
};

struct LogicRule {
    bool enabled;
    char name[24];
    LogicConditionType cond1_type;
    uint8_t cond1_target; // 1-16
    LogicOperator logic_op;
    LogicConditionType cond2_type;
    uint8_t cond2_target; // 1-16
    InputAction action;
    uint8_t action_target; // 1-16
};

struct LogicRuleRuntime {
    bool last_eval_result;
};


