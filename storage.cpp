#include "storage.h"

StorageManager Storage;

StorageManager::StorageManager() {}

void StorageManager::begin() {
    prefs.begin("sys_sec", false);
    bool has_sec = prefs.isKey("configured");
    prefs.end();

    if (!has_sec) {
        initDefaults();
    }
}

void StorageManager::initDefaults() {
    SecurityConfig sec = {};
    sec.is_configured = false;
    strncpy(sec.username, "admin", sizeof(sec.username));
    sec.session_timeout_s = DEFAULT_SESSION_EXP;
    sec.lockout_attempts = MAX_FAILED_LOGINS;
    sec.lockout_time_s = LOCKOUT_DURATION_S;
    saveSecurity(sec);

    NetworkConfig net = {};
    net.dhcp = true;
    strncpy(net.ip, "192.168.1.200", sizeof(net.ip));
    strncpy(net.gateway, "192.168.1.1", sizeof(net.gateway));
    strncpy(net.subnet, "255.255.255.0", sizeof(net.subnet));
    strncpy(net.dns, "192.168.1.1", sizeof(net.dns));
    strncpy(net.hostname, "wt32-relays", sizeof(net.hostname));
    saveNetwork(net);

    MqttConfig mqtt = {};
    mqtt.enabled = false;
    mqtt.use_ssl = false;
    mqtt.ssl_mode = 0;
    strncpy(mqtt.host, "192.168.1.50", sizeof(mqtt.host));
    mqtt.port = 1883;
    strncpy(mqtt.client_id, "wt32_controller", sizeof(mqtt.client_id));
    strncpy(mqtt.base_topic, "wt32", sizeof(mqtt.base_topic));
    mqtt.ha_discovery = true;
    saveMqtt(mqtt);

    TimeConfig time_cfg = {};
    strncpy(time_cfg.ntp_server, "pool.ntp.org", sizeof(time_cfg.ntp_server));
    time_cfg.gmt_offset_s = 14400;
    time_cfg.dst_offset_s = 0;
    time_cfg.latitude = DEFAULT_LATITUDE;
    time_cfg.longitude = DEFAULT_LONGITUDE;
    saveTime(time_cfg);

    EmergencyConfig emg = {};
    emg.trigger_inputs_mask = 0;
    emg.turn_off_all = true;
    emg.target_relays_mask = 0xFFFF;
    emg.lock_relays = true;
    emg.manual_reset_only = true;
    emg.auto_reset_seconds = 0;
    emg.latch_mode = true;
    saveEmergency(emg);
    setEmergencyLock(false);

    InputDefaults defs = {};
    defs.debounce_ms = DEFAULT_DEBOUNCE_MS;
    defs.long_press_ms = DEFAULT_LONG_PRESS;
    defs.double_click_ms = DEFAULT_DBL_CLICK;
    defs.signal_timeout_s = DEFAULT_SIG_TIMEOUT;
    defs.timeout_action = TIMEOUT_WARN_ONLY;
    saveInputDefaults(defs);

    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        RelayConfig rc = {};
        snprintf(rc.name, sizeof(rc.name), "Relay %u", i + 1);
        rc.enabled = true;
        rc.restore_mode = RESTORE_ALWAYS_OFF;
        rc.auto_off_sec = 0;
        rc.on_delay_ms = 0;
        rc.off_delay_ms = 0;
        rc.interlock_group = 0;
        rc.manual_control_enabled = true;
        rc.mqtt_control_enabled = true;
        rc.input_control_enabled = true;
        rc.event_logging_enabled = true;
        saveRelay(i, rc);
        saveRelayLastState(i, false);
    }

    for (uint8_t i = 0; i < NUM_INPUTS; ++i) {
        InputConfig ic = {};
        snprintf(ic.name, sizeof(ic.name), "Input %u", i + 1);
        ic.enabled = true;
        ic.polarity = ACTIVE_LOW;
        ic.debounce_ms = DEFAULT_DEBOUNCE_MS;
        ic.long_press_ms = DEFAULT_LONG_PRESS;
        ic.double_click_ms = DEFAULT_DBL_CLICK;
        ic.signal_timeout_s = DEFAULT_SIG_TIMEOUT;
        ic.timeout_action = TIMEOUT_WARN_ONLY;
        ic.mode = MODE_TOGGLE;
        ic.target_relay = i + 1;
        ic.target_scene = 1;
        ic.short_action = ACT_RELAY_TOGGLE;
        ic.long_action = ACT_NONE;
        ic.double_action = ACT_NONE;
        ic.startup = STARTUP_IGNORE;
        ic.is_emergency = false;
        ic.emergency_relay = 0;
        ic.dash_visible = true;
        ic.mqtt_pub = true;
        ic.log_enabled = true;
        saveInput(i, ic);
    }

    for (uint8_t i = 0; i < NUM_SCENES; ++i) {
        SceneConfig sc = {};
        snprintf(sc.name, sizeof(sc.name), "Scene %u", i + 1);
        for (uint8_t r = 0; r < NUM_RELAYS; ++r) {
            sc.actions[r] = SCENE_RELAY_IGNORE;
        }
        saveScene(i, sc);
    }

    for (uint8_t i = 0; i < NUM_SCHEDULES; ++i) {
        ScheduleItem si = {};
        snprintf(si.name, sizeof(si.name), "Timer %u", i + 1);
        si.enabled = false;
        si.days_mask = 0x7F;
        si.hour = 0;
        si.minute = 0;
        si.action = ACT_NONE;
        si.target_relay = 1;
        si.target_scene = 1;
        saveSchedule(i, si);
    }

    WifiConfig wifi = {};
    wifi.sta_enabled = false;
    wifi.sta_ssid[0] = '\0';
    wifi.sta_pass[0] = '\0';
    wifi.ap_fallback_enabled = true;
    strncpy(wifi.ap_ssid, "WT32-Relay-Setup", sizeof(wifi.ap_ssid));
    strncpy(wifi.ap_pass, "12345678", sizeof(wifi.ap_pass));
    saveWifi(wifi);

    PingWatchdogConfig wd = {};
    wd.enabled = false;
    strncpy(wd.host, "8.8.8.8", sizeof(wd.host));
    wd.port = 53;
    wd.check_interval_s = 60;
    wd.fail_threshold = 3;
    wd.target_relay = 1;
    wd.power_off_duration_s = 10;
    wd.cooldown_s = 300;
    savePingWatchdog(wd);
}

void StorageManager::loadSecurity(SecurityConfig &cfg) {
    prefs.begin("sys_sec", true);
    cfg.is_configured = prefs.getBool("configured", false);
    String u = prefs.getString("user", "admin");
    strncpy(cfg.username, u.c_str(), sizeof(cfg.username));
    String s = prefs.getString("salt", "");
    strncpy(cfg.salt, s.c_str(), sizeof(cfg.salt));
    String h = prefs.getString("hash", "");
    strncpy(cfg.pass_hash, h.c_str(), sizeof(cfg.pass_hash));
    cfg.session_timeout_s = prefs.getUInt("timeout", DEFAULT_SESSION_EXP);
    cfg.lockout_attempts = (uint8_t)prefs.getUChar("lock_att", MAX_FAILED_LOGINS);
    cfg.lockout_time_s = prefs.getUInt("lock_time", LOCKOUT_DURATION_S);
    prefs.end();
}

void StorageManager::saveSecurity(const SecurityConfig &cfg) {
    prefs.begin("sys_sec", false);
    prefs.putBool("configured", cfg.is_configured);
    prefs.putString("user", cfg.username);
    prefs.putString("salt", cfg.salt);
    prefs.putString("hash", cfg.pass_hash);
    prefs.putUInt("timeout", cfg.session_timeout_s);
    prefs.putUChar("lock_att", cfg.lockout_attempts);
    prefs.putUInt("lock_time", cfg.lockout_time_s);
    prefs.end();
}

void StorageManager::loadNetwork(NetworkConfig &cfg) {
    prefs.begin("sys_net", true);
    cfg.dhcp = prefs.getBool("dhcp", true);
    String ip = prefs.getString("ip", "192.168.1.200");
    strncpy(cfg.ip, ip.c_str(), sizeof(cfg.ip));
    String gw = prefs.getString("gw", "192.168.1.1");
    strncpy(cfg.gateway, gw.c_str(), sizeof(cfg.gateway));
    String sub = prefs.getString("mask", "255.255.255.0");
    strncpy(cfg.subnet, sub.c_str(), sizeof(cfg.subnet));
    String dns = prefs.getString("dns", "192.168.1.1");
    strncpy(cfg.dns, dns.c_str(), sizeof(cfg.dns));
    String host = prefs.getString("host", "wt32-relays");
    strncpy(cfg.hostname, host.c_str(), sizeof(cfg.hostname));
    prefs.end();
}

void StorageManager::saveNetwork(const NetworkConfig &cfg) {
    prefs.begin("sys_net", false);
    prefs.putBool("dhcp", cfg.dhcp);
    prefs.putString("ip", cfg.ip);
    prefs.putString("gw", cfg.gateway);
    prefs.putString("mask", cfg.subnet);
    prefs.putString("dns", cfg.dns);
    prefs.putString("host", cfg.hostname);
    prefs.end();
}

void StorageManager::loadMqtt(MqttConfig &cfg) {
    prefs.begin("sys_mqtt", true);
    cfg.enabled = prefs.getBool("enabled", false);
    cfg.use_ssl = prefs.getBool("use_ssl", false);
    cfg.ssl_mode = prefs.getUChar("ssl_mode", 0);
    String host = prefs.getString("host", "192.168.1.50");
    strncpy(cfg.host, host.c_str(), sizeof(cfg.host));
    cfg.port = prefs.getUShort("port", 1883);
    String user = prefs.getString("user", "");
    strncpy(cfg.user, user.c_str(), sizeof(cfg.user));
    String pass = prefs.getString("pass", "");
    strncpy(cfg.pass, pass.c_str(), sizeof(cfg.pass));
    String cid = prefs.getString("cid", "wt32_controller");
    strncpy(cfg.client_id, cid.c_str(), sizeof(cfg.client_id));
    String topic = prefs.getString("topic", "wt32");
    strncpy(cfg.base_topic, topic.c_str(), sizeof(cfg.base_topic));
    cfg.ha_discovery = prefs.getBool("ha_disc", true);
    prefs.end();
}

void StorageManager::saveMqtt(const MqttConfig &cfg) {
    prefs.begin("sys_mqtt", false);
    prefs.putBool("enabled", cfg.enabled);
    prefs.putBool("use_ssl", cfg.use_ssl);
    prefs.putUChar("ssl_mode", cfg.ssl_mode);
    prefs.putString("host", cfg.host);
    prefs.putUShort("port", cfg.port);
    prefs.putString("user", cfg.user);
    prefs.putString("pass", cfg.pass);
    prefs.putString("cid", cfg.client_id);
    prefs.putString("topic", cfg.base_topic);
    prefs.putBool("ha_disc", cfg.ha_discovery);
    prefs.end();
}

void StorageManager::loadTime(TimeConfig &cfg) {
    prefs.begin("sys_time", true);
    String ntp = prefs.getString("ntp", "pool.ntp.org");
    strncpy(cfg.ntp_server, ntp.c_str(), sizeof(cfg.ntp_server));
    cfg.gmt_offset_s = prefs.getInt("gmt", 14400); // Default to UTC+4
    cfg.dst_offset_s = prefs.getInt("dst", 0);
    cfg.latitude = prefs.getFloat("lat", DEFAULT_LATITUDE);
    cfg.longitude = prefs.getFloat("lon", DEFAULT_LONGITUDE);
    prefs.end();
}

void StorageManager::saveTime(const TimeConfig &cfg) {
    prefs.begin("sys_time", false);
    prefs.putString("ntp", cfg.ntp_server);
    prefs.putInt("gmt", cfg.gmt_offset_s);
    prefs.putInt("dst", cfg.dst_offset_s);
    prefs.putFloat("lat", cfg.latitude);
    prefs.putFloat("lon", cfg.longitude);
    prefs.end();
}


void StorageManager::loadEmergency(EmergencyConfig &cfg, bool &locked) {
    prefs.begin("sys_emg", true);
    locked = prefs.getBool("locked", false);
    cfg.trigger_inputs_mask = prefs.getUShort("trig_mask", 0);
    cfg.turn_off_all = prefs.getBool("off_all", true);
    cfg.target_relays_mask = prefs.getUShort("tgt_mask", 0xFFFF);
    cfg.lock_relays = prefs.getBool("lock_rel", true);
    cfg.manual_reset_only = prefs.getBool("man_rst", true);
    cfg.auto_reset_seconds = prefs.getUInt("auto_s", 0);
    cfg.latch_mode = prefs.getBool("latch", true);
    prefs.end();
}

void StorageManager::saveEmergency(const EmergencyConfig &cfg) {
    prefs.begin("sys_emg", false);
    prefs.putUShort("trig_mask", cfg.trigger_inputs_mask);
    prefs.putBool("off_all", cfg.turn_off_all);
    prefs.putUShort("tgt_mask", cfg.target_relays_mask);
    prefs.putBool("lock_rel", cfg.lock_relays);
    prefs.putBool("man_rst", cfg.manual_reset_only);
    prefs.putUInt("auto_s", cfg.auto_reset_seconds);
    prefs.putBool("latch", cfg.latch_mode);
    prefs.end();
}

void StorageManager::setEmergencyLock(bool locked) {
    prefs.begin("sys_emg", false);
    prefs.putBool("locked", locked);
    prefs.end();
}

void StorageManager::loadInputDefaults(InputDefaults &defs) {
    prefs.begin("sys_gdef", true);
    defs.debounce_ms = prefs.getUShort("deb", DEFAULT_DEBOUNCE_MS);
    defs.long_press_ms = prefs.getUShort("long", DEFAULT_LONG_PRESS);
    defs.double_click_ms = prefs.getUShort("dbl", DEFAULT_DBL_CLICK);
    defs.signal_timeout_s = prefs.getUShort("tout", DEFAULT_SIG_TIMEOUT);
    defs.timeout_action = (SignalTimeoutAction)prefs.getUChar("tact", TIMEOUT_WARN_ONLY);
    prefs.end();
}

void StorageManager::saveInputDefaults(const InputDefaults &defs) {
    prefs.begin("sys_gdef", false);
    prefs.putUShort("deb", defs.debounce_ms);
    prefs.putUShort("long", defs.long_press_ms);
    prefs.putUShort("dbl", defs.double_click_ms);
    prefs.putUShort("tout", defs.signal_timeout_s);
    prefs.putUChar("tact", (uint8_t)defs.timeout_action);
    prefs.end();
}

void StorageManager::loadRelay(uint8_t idx, RelayConfig &cfg, bool &last_state) {
    char key[16];
    snprintf(key, sizeof(key), "r_cfg_%u", idx);
    prefs.begin("sys_relay", true);
    size_t sz = prefs.getBytes(key, &cfg, sizeof(RelayConfig));
    if (sz != sizeof(RelayConfig)) {
        snprintf(cfg.name, sizeof(cfg.name), "Relay %u", idx + 1);
        cfg.enabled = true;
        cfg.restore_mode = RESTORE_ALWAYS_OFF;
        cfg.auto_off_sec = 0;
        cfg.on_delay_ms = 0;
        cfg.off_delay_ms = 0;
        cfg.interlock_group = 0;
        cfg.manual_control_enabled = true;
        cfg.mqtt_control_enabled = true;
        cfg.input_control_enabled = true;
        cfg.event_logging_enabled = true;
        cfg.rated_power_w = 0;
        cfg.device_icon = ICON_LIGHT;
        strncpy(cfg.room, "General", sizeof(cfg.room));
    }

    snprintf(key, sizeof(key), "r_last_%u", idx);
    last_state = prefs.getBool(key, false);
    prefs.end();
}

void StorageManager::saveRelay(uint8_t idx, const RelayConfig &cfg) {
    char key[16];
    snprintf(key, sizeof(key), "r_cfg_%u", idx);
    prefs.begin("sys_relay", false);
    prefs.putBytes(key, &cfg, sizeof(RelayConfig));
    prefs.end();
}

void StorageManager::saveRelayLastState(uint8_t idx, bool state) {
    char key[16];
    snprintf(key, sizeof(key), "r_last_%u", idx);
    prefs.begin("sys_relay", false);
    prefs.putBool(key, state);
    prefs.end();
}

void StorageManager::loadInput(uint8_t idx, InputConfig &cfg) {
    char key[16];
    snprintf(key, sizeof(key), "in_cfg_%u", idx);
    prefs.begin("sys_input", true);
    size_t sz = prefs.getBytes(key, &cfg, sizeof(InputConfig));
    if (sz != sizeof(InputConfig)) {
        snprintf(cfg.name, sizeof(cfg.name), "Input %u", idx + 1);
        cfg.enabled = true;
        cfg.polarity = ACTIVE_LOW;
        cfg.debounce_ms = DEFAULT_DEBOUNCE_MS;
        cfg.long_press_ms = DEFAULT_LONG_PRESS;
        cfg.double_click_ms = DEFAULT_DBL_CLICK;
        cfg.signal_timeout_s = DEFAULT_SIG_TIMEOUT;
        cfg.timeout_action = TIMEOUT_WARN_ONLY;
        cfg.mode = MODE_TOGGLE;
        cfg.target_relay = idx + 1;
        cfg.target_scene = 1;
        cfg.short_action = ACT_RELAY_TOGGLE;
        cfg.long_action = ACT_NONE;
        cfg.double_action = ACT_NONE;
        cfg.startup = STARTUP_IGNORE;
        cfg.is_emergency = false;
        cfg.emergency_relay = 0;
        cfg.dash_visible = true;
        cfg.mqtt_pub = true;
        cfg.log_enabled = true;
    }
    prefs.end();
}

void StorageManager::saveInput(uint8_t idx, const InputConfig &cfg) {
    char key[16];
    snprintf(key, sizeof(key), "in_cfg_%u", idx);
    prefs.begin("sys_input", false);
    prefs.putBytes(key, &cfg, sizeof(InputConfig));
    prefs.end();
}

void StorageManager::loadScene(uint8_t idx, SceneConfig &cfg) {
    char key[16];
    snprintf(key, sizeof(key), "sc_cfg_%u", idx);
    prefs.begin("sys_scene", true);
    size_t sz = prefs.getBytes(key, &cfg, sizeof(SceneConfig));
    if (sz != sizeof(SceneConfig)) {
        snprintf(cfg.name, sizeof(cfg.name), "Scene %u", idx + 1);
        for (uint8_t r = 0; r < NUM_RELAYS; ++r) {
            cfg.actions[r] = SCENE_RELAY_IGNORE;
        }
    }
    prefs.end();
}

void StorageManager::saveScene(uint8_t idx, const SceneConfig &cfg) {
    char key[16];
    snprintf(key, sizeof(key), "sc_cfg_%u", idx);
    prefs.begin("sys_scene", false);
    prefs.putBytes(key, &cfg, sizeof(SceneConfig));
    prefs.end();
}

void StorageManager::loadSchedule(uint8_t idx, ScheduleItem &item) {
    char key[16];
    snprintf(key, sizeof(key), "sch_cfg_%u", idx);
    prefs.begin("sys_sched", true);
    size_t sz = prefs.getBytes(key, &item, sizeof(ScheduleItem));
    if (sz != sizeof(ScheduleItem)) {
        snprintf(item.name, sizeof(item.name), "Timer %u", idx + 1);
        item.enabled = false;
        item.days_mask = 0x7F;
        item.hour = 0;
        item.minute = 0;
        item.action = ACT_NONE;
        item.target_relay = 1;
        item.target_scene = 1;
        item.trigger_type = TRIGGER_CLOCK;
        item.offset_minutes = 0;
    }
    prefs.end();
}

void StorageManager::saveSchedule(uint8_t idx, const ScheduleItem &item) {
    char key[16];
    snprintf(key, sizeof(key), "sch_cfg_%u", idx);
    prefs.begin("sys_sched", false);
    prefs.putBytes(key, &item, sizeof(ScheduleItem));
    prefs.end();
}

void StorageManager::loadRelayEnergy(uint8_t idx, RelayEnergyStats &stats) {
    char key[16];
    snprintf(key, sizeof(key), "r_nrg_%u", idx);
    prefs.begin("sys_energy", true);
    size_t sz = prefs.getBytes(key, &stats, sizeof(RelayEnergyStats));
    if (sz != sizeof(RelayEnergyStats)) {
        stats.total_cycles = 0;
        stats.total_on_seconds = 0;
    }
    prefs.end();
}

void StorageManager::saveRelayEnergy(uint8_t idx, const RelayEnergyStats &stats) {
    char key[16];
    snprintf(key, sizeof(key), "r_nrg_%u", idx);
    prefs.begin("sys_energy", false);
    prefs.putBytes(key, &stats, sizeof(RelayEnergyStats));
    prefs.end();
}

void StorageManager::loadLogicRule(uint8_t idx, LogicRule &rule) {
    char key[16];
    snprintf(key, sizeof(key), "rule_%u", idx);
    prefs.begin("sys_logic", true);
    size_t sz = prefs.getBytes(key, &rule, sizeof(LogicRule));
    if (sz != sizeof(LogicRule)) {
        rule.enabled = false;
        snprintf(rule.name, sizeof(rule.name), "Rule %u", idx + 1);
        rule.cond1_type = COND_NONE;
        rule.cond1_target = 1;
        rule.logic_op = LOGIC_OP_NONE;
        rule.cond2_type = COND_NONE;
        rule.cond2_target = 1;
        rule.action = ACT_NONE;
        rule.action_target = 1;
    }
    prefs.end();
}

void StorageManager::saveLogicRule(uint8_t idx, const LogicRule &rule) {
    char key[16];
    snprintf(key, sizeof(key), "rule_%u", idx);
    prefs.begin("sys_logic", false);
    prefs.putBytes(key, &rule, sizeof(LogicRule));
    prefs.end();
}

void StorageManager::loadWifi(WifiConfig &cfg) {
    prefs.begin("sys_wifi", true);
    cfg.sta_enabled = prefs.getBool("sta_en", false);
    String s_ssid = prefs.getString("sta_ssid", "");
    strncpy(cfg.sta_ssid, s_ssid.c_str(), sizeof(cfg.sta_ssid));
    String s_pass = prefs.getString("sta_pass", "");
    strncpy(cfg.sta_pass, s_pass.c_str(), sizeof(cfg.sta_pass));
    cfg.ap_fallback_enabled = prefs.getBool("ap_fb", true);
    String a_ssid = prefs.getString("ap_ssid", "WT32-Relay-Setup");
    strncpy(cfg.ap_ssid, a_ssid.c_str(), sizeof(cfg.ap_ssid));
    String a_pass = prefs.getString("ap_pass", "12345678");
    strncpy(cfg.ap_pass, a_pass.c_str(), sizeof(cfg.ap_pass));
    prefs.end();

    if (strlen(cfg.ap_ssid) == 0) {
        strncpy(cfg.ap_ssid, "WT32-Relay-Setup", sizeof(cfg.ap_ssid));
    }
    if (strlen(cfg.ap_pass) < 8 && strcmp(cfg.ap_pass, "none") != 0 && strcmp(cfg.ap_pass, "open") != 0) {
        strncpy(cfg.ap_pass, "12345678", sizeof(cfg.ap_pass));
    }
}

void StorageManager::saveWifi(const WifiConfig &cfg) {
    prefs.begin("sys_wifi", false);
    prefs.putBool("sta_en", cfg.sta_enabled);
    prefs.putString("sta_ssid", cfg.sta_ssid);
    prefs.putString("sta_pass", cfg.sta_pass);
    prefs.putBool("ap_fb", cfg.ap_fallback_enabled);
    prefs.putString("ap_ssid", cfg.ap_ssid);
    prefs.putString("ap_pass", cfg.ap_pass);
    prefs.end();
}

void StorageManager::loadPingWatchdog(PingWatchdogConfig &cfg) {
    prefs.begin("sys_watchdog", true);
    cfg.enabled = prefs.getBool("enabled", false);
    String h = prefs.getString("host", "8.8.8.8");
    strncpy(cfg.host, h.c_str(), sizeof(cfg.host));
    cfg.port = prefs.getUShort("port", 53);
    cfg.check_interval_s = prefs.getUShort("interval", 60);
    cfg.fail_threshold = prefs.getUChar("fails", 3);
    cfg.target_relay = prefs.getUChar("relay", 1);
    cfg.power_off_duration_s = prefs.getUShort("off_sec", 10);
    cfg.cooldown_s = prefs.getUShort("cooldown", 300);
    prefs.end();
}

void StorageManager::savePingWatchdog(const PingWatchdogConfig &cfg) {
    prefs.begin("sys_watchdog", false);
    prefs.putBool("enabled", cfg.enabled);
    prefs.putString("host", cfg.host);
    prefs.putUShort("port", cfg.port);
    prefs.putUShort("interval", cfg.check_interval_s);
    prefs.putUChar("fails", cfg.fail_threshold);
    prefs.putUChar("relay", cfg.target_relay);
    prefs.putUShort("off_sec", cfg.power_off_duration_s);
    prefs.putUShort("cooldown", cfg.cooldown_s);
    prefs.end();
}

void StorageManager::factoryReset() {
    const char *namespaces[] = {
        "sys_sec", "sys_net", "sys_mqtt", "sys_time",
        "sys_emg", "sys_gdef", "sys_relay", "sys_input", "sys_scene", "sys_sched",
        "sys_wifi", "sys_watchdog", "sys_energy", "sys_logic"
    };
    for (const char *ns : namespaces) {
        prefs.begin(ns, false);
        prefs.clear();
        prefs.end();
    }
    initDefaults();
}


