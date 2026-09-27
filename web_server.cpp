#include "web_server.h"
#include "web_assets.h"
#include "storage.h"
#include "relay_subsystem.h"
#include "input_subsystem.h"
#include "emergency_subsystem.h"
#include "scene_subsystem.h"
#include "network_manager.h"
#include "mqtt_manager.h"
#include "auth_manager.h"
#include "timer_manager.h"
#include "event_log.h"
#include "hardware_io.h"
#include "schedule_subsystem.h"
#include "ntp_manager.h"
#include "ping_watchdog.h"
#include "astro_clock.h"
#include "logic_engine.h"
#include <esp_task_wdt.h>

#include <ArduinoJson.h>

WebServerManager WebServerMgr;

static void onWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    WebServerMgr.handleWebSocketEvent(num, type, payload, length);
}

WebServerManager::WebServerManager()
    : server(HTTP_PORT), wsServer(WS_PORT), last_broadcast_ms(0) {}

void WebServerManager::begin() {
    const char *headerkeys[] = {"Cookie", "X-Session", "Authorization"};
    server.collectHeaders(headerkeys, 3);

    setupRoutes();
    server.begin();

    wsServer.begin();
    wsServer.onEvent(onWsEvent);

    EventLog.log(SUBSYS_SYSTEM, "Web server started on port %u, WS on port %u", HTTP_PORT, WS_PORT);
}

void WebServerManager::setupRoutes() {
    server.on("/", HTTP_GET, [this]() { handleRoot(); });
    server.on("/manifest.json", HTTP_GET, [this]() { handleManifest(); });
    server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
    server.on("/api/setup", HTTP_POST, [this]() { handleSetup(); });
    server.on("/api/login", HTTP_POST, [this]() { handleLogin(); });
    server.on("/api/logout", HTTP_POST, [this]() { handleLogout(); });

    server.on("/api/relay/toggle", HTTP_POST, [this]() { handleRelayToggle(); });
    server.on("/api/relay/all_on", HTTP_POST, [this]() { handleRelayAllOn(); });
    server.on("/api/relay/all_off", HTTP_POST, [this]() { handleRelayAllOff(); });
    server.on("/api/relay/config", HTTP_GET, [this]() { handleRelayConfigGet(); });
    server.on("/api/relay/config", HTTP_POST, [this]() { handleRelayConfigPost(); });
    server.on("/api/interlock/config", HTTP_GET, [this]() { handleInterlockConfigGet(); });
    server.on("/api/interlock/config", HTTP_POST, [this]() { handleInterlockConfigPost(); });

    server.on("/api/input/config", HTTP_GET, [this]() { handleInputConfigGet(); });
    server.on("/api/input/config", HTTP_POST, [this]() { handleInputConfigPost(); });
    server.on("/api/input/defaults", HTTP_POST, [this]() { handleInputDefaultsPost(); });
    server.on("/api/input/defaults/apply_all", HTTP_POST, [this]() { handleInputDefaultsApplyAll(); });
    server.on("/api/input/defaults/reset", HTTP_POST, [this]() { handleInputDefaultsReset(); });

    server.on("/api/emergency/reset", HTTP_POST, [this]() { handleEmergencyReset(); });

    server.on("/api/scene/config", HTTP_GET, [this]() { handleSceneConfigGet(); });
    server.on("/api/scene/config", HTTP_POST, [this]() { handleSceneConfigPost(); });
    server.on("/api/scene/activate", HTTP_POST, [this]() { handleSceneActivate(); });

    server.on("/api/network/config", HTTP_GET, [this]() { handleNetworkConfigGet(); });
    server.on("/api/network/config", HTTP_POST, [this]() { handleNetworkConfigPost(); });

    server.on("/api/mqtt/config", HTTP_GET, [this]() { handleMqttConfigGet(); });
    server.on("/api/mqtt/config", HTTP_POST, [this]() { handleMqttConfigPost(); });

    server.on("/api/security/config", HTTP_GET, [this]() { handleSecurityConfigGet(); });
    server.on("/api/security/credentials", HTTP_POST, [this]() { handleSecurityCredentialsPost(); });
    server.on("/api/security/policy", HTTP_POST, [this]() { handleSecurityPolicyPost(); });

    server.on("/api/logs", HTTP_GET, [this]() { handleLogsGet(); });
    server.on("/api/logs/clear", HTTP_POST, [this]() { handleLogsClear(); });

    server.on("/api/system/info", HTTP_GET, [this]() { handleSystemInfo(); });
    server.on("/api/system/reboot", HTTP_POST, [this]() { handleReboot(); });
    server.on("/api/system/reset", HTTP_POST, [this]() { handleFactoryReset(); });
    server.on("/api/system/update", HTTP_POST, [this]() { handleUpdateDone(); }, [this]() { handleUpdateUpload(); });
    server.on("/api/system/backup", HTTP_GET, [this]() { handleBackup(); });
    server.on("/api/system/restore", HTTP_POST, [this]() { handleRestore(); });

    server.on("/api/schedule/config", HTTP_GET, [this]() { handleScheduleGet(); });
    server.on("/api/schedule/config", HTTP_POST, [this]() { handleSchedulePost(); });

    server.on("/api/wifi/config", HTTP_GET, [this]() { handleWifiConfigGet(); });
    server.on("/api/wifi/config", HTTP_POST, [this]() { handleWifiConfigPost(); });
    server.on("/api/wifi/scan", HTTP_GET, [this]() { handleWifiScanGet(); });

    server.on("/api/watchdog/config", HTTP_GET, [this]() { handleWatchdogConfigGet(); });
    server.on("/api/watchdog/config", HTTP_POST, [this]() { handleWatchdogConfigPost(); });
    server.on("/api/watchdog/test", HTTP_POST, [this]() { handleWatchdogTestPost(); });

    server.on("/api/energy/status", HTTP_GET, [this]() { handleEnergyStatus(); });
    server.on("/api/energy/reset", HTTP_POST, [this]() { handleEnergyReset(); });

    server.on("/api/logic/config", HTTP_GET, [this]() { handleLogicConfigGet(); });
    server.on("/api/logic/config", HTTP_POST, [this]() { handleLogicConfigPost(); });

    server.on("/api/time/config", HTTP_GET, [this]() { handleTimeConfigGet(); });
    server.on("/api/time/config", HTTP_POST, [this]() { handleTimeConfigPost(); });
    server.on("/api/time/set", HTTP_POST, [this]() { handleTimeSetPost(); });
    server.on("/api/time/sync_ntp", HTTP_POST, [this]() { handleTimeSyncNtpPost(); });
}


String WebServerManager::extractSessionToken() {
    if (server.hasArg("token")) {
        String token = server.arg("token");
        token.trim();
        if (token.length() == 32) return token;
    }
    if (server.hasHeader("X-Session")) {
        String token = server.header("X-Session");
        token.trim();
        if (token.length() == 32) return token;
    }
    if (server.hasHeader("Authorization")) {
        String auth = server.header("Authorization");
        if (auth.startsWith("Bearer ")) {
            String token = auth.substring(7);
            token.trim();
            if (token.length() == 32) return token;
        }
    }
    if (server.hasHeader("Cookie")) {
        String cookie = server.header("Cookie");
        int idx = cookie.indexOf("session=");
        if (idx >= 0) {
            String token = cookie.substring(idx + 8);
            int end_idx = token.indexOf(';');
            if (end_idx > 0) token = token.substring(0, end_idx);
            token.trim();
            if (token.length() == 32) return token;
        }
    }
    return String("");
}

bool WebServerManager::checkAuth() {
    if (!Auth.isConfigured()) return false;
    String token = extractSessionToken();
    return Auth.validateSession(token);
}

void WebServerManager::handleRoot() {
    server.send_P(200, "text/html", INDEX_HTML);
}

void WebServerManager::buildStatusJson(String &output) {
    bool is_auth = checkAuth();

    output.reserve(3072);
    output = "{\"type\":\"state\",";
    output += "\"configured\":" + String(Auth.isConfigured() ? "true" : "false") + ",";
    output += "\"authenticated\":" + String(is_auth ? "true" : "false") + ",";

    output += "\"temp\":" + String(temperatureRead(), 1) + ",";
    output += "\"uptime\":" + String(millis() / 1000UL) + ",";
    char time_buf[32];
    getFormattedTime(time_buf, sizeof(time_buf));
    output += "\"time\":\"" + String(time_buf) + "\",";
    output += "\"ntp_synced\":" + String(Ntp.isSynced() ? "true" : "false") + ",";
    output += "\"sunrise\":\"" + Astro.getSunriseFormatted() + "\",";
    output += "\"sunset\":\"" + Astro.getSunsetFormatted() + "\",";
    output += "\"is_night\":" + String(Astro.isNight() ? "true" : "false") + ",";
    output += "\"total_power_w\":" + String(Relays.getTotalSystemPowerW(), 1) + ",";
    output += "\"total_kwh\":" + String(Relays.getTotalSystemKWh(), 3) + ",";
    output += "\"i2c_err\":" + String(HW.getErrorCount()) + ",";

    output += "\"emg\":{";
    output += "\"active\":" + String(Emergency.isEmergencyActive() ? "true" : "false") + ",";
    output += "\"locked\":" + String(Emergency.isRelaysLocked() ? "true" : "false") + ",";
    output += "\"source\":\"" + String(Emergency.getRuntime().trigger_source) + "\"},";

    output += "\"net\":{";
    output += "\"connected\":" + String(Network.isConnected() ? "true" : "false") + ",";
    output += "\"ip\":\"" + Network.getIp() + "\",";
    output += "\"mac\":\"" + Network.getMac() + "\",";
    output += "\"speed\":" + String(Network.getLinkSpeed()) + "},";

    output += "\"mqtt\":{";
    output += "\"enabled\":" + String(Mqtt.isEnabled() ? "true" : "false") + ",";
    output += "\"connected\":" + String(Mqtt.isConnected() ? "true" : "false") + "},";

    const WifiRuntime &wr = Network.getWifiRuntime();
    const WifiConfig &wc = Network.getWifiConfig();
    output += "\"wifi\":{";
    output += "\"sta_enabled\":" + String(wc.sta_enabled ? "true" : "false") + ",";
    output += "\"sta_connected\":" + String(wr.sta_connected ? "true" : "false") + ",";
    output += "\"sta_ip\":\"" + String(wr.sta_ip) + "\",";
    output += "\"sta_rssi\":" + String(wr.sta_rssi) + ",";
    output += "\"ap_active\":" + String(wr.ap_active ? "true" : "false") + ",";
    output += "\"ap_ip\":\"" + String(wr.ap_ip) + "\",";
    output += "\"ap_clients\":" + String(wr.ap_clients) + "},";

    const PingWatchdogConfig &wdc = PingWatchdog.getConfig();
    const PingWatchdogRuntime &wdr = PingWatchdog.getRuntime();
    output += "\"watchdog\":{";
    output += "\"enabled\":" + String(wdc.enabled ? "true" : "false") + ",";
    output += "\"host\":\"" + String(wdc.host) + "\",";
    output += "\"port\":" + String(wdc.port) + ",";
    output += "\"current_fails\":" + String(wdr.current_fails) + ",";
    output += "\"fail_threshold\":" + String(wdc.fail_threshold) + ",";
    output += "\"total_reboots\":" + String(wdr.total_reboots) + ",";
    output += "\"is_power_cycling\":" + String(wdr.is_power_cycling ? "true" : "false") + ",";
    output += "\"in_cooldown\":" + String(wdr.in_cooldown ? "true" : "false") + ",";
    output += "\"last_result\":\"" + String(wdr.last_result) + "\"},";

    output += "\"relays\":[";
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        if (i > 0) output += ",";
        const RelayConfig &rc = Relays.getConfig(i);
        const RelayRuntime &rr = Relays.getRuntime(i);
        uint32_t rem = TimerMgr.getRemainingSeconds(i);

        output += "{\"name\":\"";
        output += rc.name;
        output += "\",\"state\":";
        output += rr.state ? "true" : "false";
        output += ",\"enabled\":";
        output += rc.enabled ? "true" : "false";
        output += ",\"group\":";
        output += String(rc.interlock_group);
        output += ",\"auto_off_sec\":";
        output += String(rc.auto_off_sec);
        output += ",\"timer_left\":";
        output += String(rem);
        output += ",\"on_delay_ms\":";
        output += String(rc.on_delay_ms);
        output += ",\"off_delay_ms\":";
        output += String(rc.off_delay_ms);
        output += ",\"power_w\":";
        output += String(rc.rated_power_w);
        output += ",\"device_icon\":";
        output += String(rc.device_icon);
        output += ",\"room\":\"";
        output += rc.room;
        output += "\"}";
    }
    output += "],";


    output += "\"inputs\":[";
    for (uint8_t i = 0; i < NUM_INPUTS; ++i) {
        if (i > 0) output += ",";
        const InputConfig &ic = Inputs.getConfig(i);
        const InputRuntime &ir = Inputs.getRuntime(i);

        output += "{\"name\":\"";
        output += ic.name;
        output += "\",\"active\":";
        output += ir.logical_active ? "true" : "false";
        output += ",\"timeout\":";
        output += ir.timeout_warned ? "true" : "false";
        output += ",\"mode\":";
        output += String((uint8_t)ic.mode);
        output += ",\"target_relay\":";
        output += String(ic.target_relay);
        output += ",\"last_evt\":\"";
        output += ir.last_event;
        output += "\"}";
    }
    output += "],";

    output += "\"recent_logs\":";
    String logsJson;
    EventLog.getRecentJson(logsJson, 8);
    output += logsJson;

    output += "}";
}

void WebServerManager::handleStatus() {
    String json;
    buildStatusJson(json);
    server.send(200, "application/json", json);
}

void WebServerManager::handleSetup() {
    if (Auth.isConfigured()) {
        server.send(400, "application/json", "{\"error\":\"Already configured\"}");
        return;
    }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(512);
#endif
    deserializeJson(doc, server.arg("plain"));

    const char *user = doc["user"] | "";
    const char *pass = doc["pass"] | "";
    uint32_t timeout = doc["timeout"] | DEFAULT_SESSION_EXP;

    if (Auth.setupInitialAdmin(user, pass, timeout)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Setup failed\"}");
    }
}

void WebServerManager::handleLogin() {
#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(512);
#endif
    deserializeJson(doc, server.arg("plain"));

    const char *user = doc["user"] | "";
    const char *pass = doc["pass"] | "";

    String token, err;
    if (Auth.authenticate(user, pass, token, err)) {
        server.sendHeader("Set-Cookie", "session=" + token + "; Path=/; HttpOnly; SameSite=Strict");
        String resp = "{\"success\":true,\"token\":\"" + token + "\"}";
        server.send(200, "application/json", resp);
    } else {
        String resp = "{\"success\":false,\"error\":\"" + err + "\"}";
        server.send(401, "application/json", resp);
    }
}

void WebServerManager::handleLogout() {
    String token = extractSessionToken();
    Auth.logout(token);
    server.sendHeader("Set-Cookie", "session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleRelayToggle() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(256);
#endif
    deserializeJson(doc, server.arg("plain"));

    int idx = doc["index"] | -1;
    if (idx >= 0 && idx < NUM_RELAYS) {
        Relays.toggleRelay(idx, SRC_WEB);
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid index\"}");
    }
}

void WebServerManager::handleRelayAllOn() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
    Relays.allRelaysOn(SRC_WEB);
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleRelayAllOff() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
    Relays.allRelaysOff(SRC_WEB);
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleRelayConfigGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    String out = "[";
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        if (i > 0) out += ",";
        const RelayConfig &rc = Relays.getConfig(i);
        out += "{\"name\":\"";
        out += rc.name;
        out += "\",\"enabled\":";
        out += rc.enabled ? "true" : "false";
        out += ",\"restore_mode\":";
        out += String((uint8_t)rc.restore_mode);
        out += ",\"auto_off_sec\":";
        out += String(rc.auto_off_sec);
        out += ",\"on_delay_ms\":";
        out += String(rc.on_delay_ms);
        out += ",\"off_delay_ms\":";
        out += String(rc.off_delay_ms);
        out += ",\"interlock_group\":";
        out += String(rc.interlock_group);
        out += ",\"manual_control_enabled\":";
        out += rc.manual_control_enabled ? "true" : "false";
        out += ",\"mqtt_control_enabled\":";
        out += rc.mqtt_control_enabled ? "true" : "false";
        out += ",\"input_control_enabled\":";
        out += rc.input_control_enabled ? "true" : "false";
        out += ",\"event_logging_enabled\":";
        out += rc.event_logging_enabled ? "true" : "false";
        out += ",\"rated_power_w\":";
        out += String(rc.rated_power_w);
        out += ",\"device_icon\":";
        out += String(rc.device_icon);
        out += ",\"room\":\"";
        out += rc.room;
        out += "\"}";
    }
    out += "]";
    server.send(200, "application/json", out);
}

void WebServerManager::handleRelayConfigPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(512);
#endif
    deserializeJson(doc, server.arg("plain"));

    int idx = doc["index"] | -1;
    if (idx >= 0 && idx < NUM_RELAYS) {
        RelayConfig rc = Relays.getConfig(idx);
        if (doc.containsKey("name")) {
            const char *name = doc["name"] | rc.name;
            strncpy(rc.name, name, sizeof(rc.name) - 1);
            rc.name[sizeof(rc.name) - 1] = '\0';
        }
        if (doc.containsKey("enabled")) rc.enabled = doc["enabled"].as<bool>();
        if (doc.containsKey("restore_mode")) rc.restore_mode = (RelayRestoreMode)doc["restore_mode"].as<uint8_t>();
        if (doc.containsKey("auto_off_sec")) rc.auto_off_sec = doc["auto_off_sec"].as<uint32_t>();
        if (doc.containsKey("on_delay_ms")) rc.on_delay_ms = doc["on_delay_ms"].as<uint32_t>();
        if (doc.containsKey("off_delay_ms")) rc.off_delay_ms = doc["off_delay_ms"].as<uint32_t>();
        if (doc.containsKey("interlock_group")) rc.interlock_group = doc["interlock_group"].as<uint8_t>();
        if (doc.containsKey("manual_control_enabled")) rc.manual_control_enabled = doc["manual_control_enabled"].as<bool>();
        if (doc.containsKey("mqtt_control_enabled")) rc.mqtt_control_enabled = doc["mqtt_control_enabled"].as<bool>();
        if (doc.containsKey("input_control_enabled")) rc.input_control_enabled = doc["input_control_enabled"].as<bool>();
        if (doc.containsKey("event_logging_enabled")) rc.event_logging_enabled = doc["event_logging_enabled"].as<bool>();
        if (doc.containsKey("rated_power_w")) rc.rated_power_w = doc["rated_power_w"].as<uint16_t>();
        if (doc.containsKey("device_icon")) rc.device_icon = doc["device_icon"].as<uint8_t>();
        if (doc.containsKey("room")) {
            const char *rm = doc["room"] | rc.room;
            strncpy(rc.room, rm, sizeof(rc.room) - 1);
            rc.room[sizeof(rc.room) - 1] = '\0';
        }

        Relays.updateConfig(idx, rc);
        broadcastState();
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid index\"}");
    }
}


void WebServerManager::handleInterlockConfigGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    String out = "[";
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        if (i > 0) out += ",";
        const RelayConfig &rc = Relays.getConfig(i);
        out += "{\"index\":" + String(i) + ",\"name\":\"" + String(rc.name) + "\",\"group\":" + String(rc.interlock_group) + "}";
    }
    out += "]";
    server.send(200, "application/json", out);
}

void WebServerManager::handleInterlockConfigPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(512);
#endif
    deserializeJson(doc, server.arg("plain"));

    if (doc.containsKey("groups")) {
        JsonArray arr = doc["groups"];
        for (uint8_t i = 0; i < NUM_RELAYS && i < arr.size(); ++i) {
            RelayConfig rc = Relays.getConfig(i);
            rc.interlock_group = arr[i].as<uint8_t>();
            Relays.updateConfig(i, rc);
        }
    } else if (doc.containsKey("index") && doc.containsKey("group")) {
        int idx = doc["index"].as<int>();
        if (idx >= 0 && idx < NUM_RELAYS) {
            RelayConfig rc = Relays.getConfig(idx);
            rc.interlock_group = doc["group"].as<uint8_t>();
            Relays.updateConfig(idx, rc);
        }
    }
    broadcastState();
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleInputConfigGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    const InputDefaults &defs = Inputs.getDefaults();

    String out = "{\"defaults\":{";
    out += "\"debounce_ms\":" + String(defs.debounce_ms) + ",";
    out += "\"long_press_ms\":" + String(defs.long_press_ms) + ",";
    out += "\"double_click_ms\":" + String(defs.double_click_ms) + ",";
    out += "\"signal_timeout_s\":" + String(defs.signal_timeout_s) + ",";
    out += "\"timeout_action\":" + String((uint8_t)defs.timeout_action) + "},";

    out += "\"inputs\":[";
    for (uint8_t i = 0; i < NUM_INPUTS; ++i) {
        if (i > 0) out += ",";
        const InputConfig &ic = Inputs.getConfig(i);
        out += "{\"name\":\"";
        out += ic.name;
        out += "\",\"enabled\":";
        out += ic.enabled ? "true" : "false";
        out += ",\"polarity\":";
        out += String((uint8_t)ic.polarity);
        out += ",\"debounce_ms\":";
        out += String(ic.debounce_ms);
        out += ",\"long_press_ms\":";
        out += String(ic.long_press_ms);
        out += ",\"double_click_ms\":";
        out += String(ic.double_click_ms);
        out += ",\"signal_timeout_s\":";
        out += String(ic.signal_timeout_s);
        out += ",\"timeout_action\":";
        out += String((uint8_t)ic.timeout_action);
        out += ",\"mode\":";
        out += String((uint8_t)ic.mode);
        out += ",\"target_relay\":";
        out += String(ic.target_relay);
        out += ",\"target_scene\":";
        out += String(ic.target_scene);
        out += ",\"short_action\":";
        out += String((uint8_t)ic.short_action);
        out += ",\"long_action\":";
        out += String((uint8_t)ic.long_action);
        out += ",\"double_action\":";
        out += String((uint8_t)ic.double_action);
        out += ",\"startup\":";
        out += String((uint8_t)ic.startup);
        out += ",\"is_emergency\":";
        out += ic.is_emergency ? "true" : "false";
        out += ",\"dash_visible\":";
        out += ic.dash_visible ? "true" : "false";
        out += "}";
    }
    out += "]}";
    server.send(200, "application/json", out);
}

void WebServerManager::handleInputConfigPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(512);
#endif
    deserializeJson(doc, server.arg("plain"));

    int idx = doc["index"] | -1;
    if (idx >= 0 && idx < NUM_INPUTS) {
        InputConfig ic = Inputs.getConfig(idx);
        if (doc.containsKey("name")) {
            const char *name = doc["name"] | ic.name;
            strncpy(ic.name, name, sizeof(ic.name) - 1);
            ic.name[sizeof(ic.name) - 1] = '\0';
        }
        if (doc.containsKey("enabled")) ic.enabled = doc["enabled"].as<bool>();
        if (doc.containsKey("polarity")) ic.polarity = (InputPolarity)doc["polarity"].as<uint8_t>();
        if (doc.containsKey("mode")) ic.mode = (InputMode)doc["mode"].as<uint8_t>();
        if (doc.containsKey("target_relay")) ic.target_relay = doc["target_relay"].as<uint8_t>();
        if (doc.containsKey("target_scene")) ic.target_scene = doc["target_scene"].as<uint8_t>();
        if (doc.containsKey("short_action")) ic.short_action = (InputAction)doc["short_action"].as<uint8_t>();
        if (doc.containsKey("long_action")) ic.long_action = (InputAction)doc["long_action"].as<uint8_t>();
        if (doc.containsKey("double_action")) ic.double_action = (InputAction)doc["double_action"].as<uint8_t>();
        if (doc.containsKey("startup")) ic.startup = (StartupBehavior)doc["startup"].as<uint8_t>();
        if (doc.containsKey("is_emergency")) ic.is_emergency = doc["is_emergency"].as<bool>();
        if (doc.containsKey("dash_visible")) ic.dash_visible = doc["dash_visible"].as<bool>();
        if (doc.containsKey("debounce_ms")) ic.debounce_ms = doc["debounce_ms"].as<uint16_t>();

        Inputs.updateConfig(idx, ic);
        broadcastState();
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid index\"}");
    }
}

void WebServerManager::handleInputDefaultsPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(256);
#endif
    deserializeJson(doc, server.arg("plain"));

    InputDefaults defs;
    defs.debounce_ms = doc["debounce_ms"] | DEFAULT_DEBOUNCE_MS;
    defs.long_press_ms = doc["long_press_ms"] | DEFAULT_LONG_PRESS;
    defs.double_click_ms = doc["double_click_ms"] | DEFAULT_DBL_CLICK;
    defs.signal_timeout_s = doc["signal_timeout_s"] | DEFAULT_SIG_TIMEOUT;
    defs.timeout_action = (SignalTimeoutAction)(doc["timeout_action"] | 0);

    Inputs.updateDefaults(defs);
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleInputDefaultsApplyAll() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
    Inputs.applyDefaultsToAll();
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleInputDefaultsReset() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(128);
#endif
    deserializeJson(doc, server.arg("plain"));

    int idx = doc["index"] | -1;
    if (idx >= 0 && idx < NUM_INPUTS) {
        Inputs.resetInputToDefaults(idx);
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid index\"}");
    }
}

void WebServerManager::handleEmergencyReset() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(128);
#endif
    deserializeJson(doc, server.arg("plain"));

    bool confirmed = doc["confirmed"] | false;
    if (confirmed && Emergency.resetEmergency(true)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Reset confirmation required\"}");
    }
}

void WebServerManager::handleSceneConfigGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    String out = "[";
    for (uint8_t i = 0; i < NUM_SCENES; ++i) {
        if (i > 0) out += ",";
        const SceneConfig &sc = Scenes.getConfig(i);
        out += "{\"name\":\"";
        out += sc.name;
        out += "\",\"actions\":[";
        for (uint8_t r = 0; r < NUM_RELAYS; ++r) {
            if (r > 0) out += ",";
            out += String((uint8_t)sc.actions[r]);
        }
        out += "]}";
    }
    out += "]";
    server.send(200, "application/json", out);
}

void WebServerManager::handleSceneConfigPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(512);
#endif
    deserializeJson(doc, server.arg("plain"));

    int idx = doc["index"] | -1;
    if (idx >= 0 && idx < NUM_SCENES) {
        SceneConfig sc;
        const char *name = doc["name"] | "";
        strncpy(sc.name, name, sizeof(sc.name) - 1);
        sc.name[sizeof(sc.name) - 1] = '\0';

        JsonArray arr = doc["actions"];
        for (uint8_t r = 0; r < NUM_RELAYS; ++r) {
            sc.actions[r] = (SceneAction)(arr[r] | 0);
        }

        Scenes.updateConfig(idx, sc);
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid index\"}");
    }
}

void WebServerManager::handleSceneActivate() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(128);
#endif
    deserializeJson(doc, server.arg("plain"));

    int idx = doc["index"] | -1;
    if (idx >= 0 && idx < NUM_SCENES) {
        Scenes.activateScene(idx);
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid index\"}");
    }
}

void WebServerManager::handleNetworkConfigGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    const NetworkConfig &cfg = Network.getConfig();
    String out = "{\"dhcp\":";
    out += cfg.dhcp ? "true" : "false";
    out += ",\"ip\":\"" + String(cfg.ip) + "\"";
    out += ",\"gateway\":\"" + String(cfg.gateway) + "\"";
    out += ",\"subnet\":\"" + String(cfg.subnet) + "\"";
    out += ",\"dns\":\"" + String(cfg.dns) + "\"";
    out += ",\"hostname\":\"" + String(cfg.hostname) + "\"}";
    server.send(200, "application/json", out);
}

void WebServerManager::handleNetworkConfigPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(512);
#endif
    deserializeJson(doc, server.arg("plain"));

    NetworkConfig cfg;
    cfg.dhcp = doc["dhcp"] | true;
    strncpy(cfg.ip, doc["ip"] | "192.168.1.200", sizeof(cfg.ip) - 1);
    strncpy(cfg.gateway, doc["gateway"] | "192.168.1.1", sizeof(cfg.gateway) - 1);
    strncpy(cfg.subnet, doc["subnet"] | "255.255.255.0", sizeof(cfg.subnet) - 1);
    strncpy(cfg.dns, doc["dns"] | "192.168.1.1", sizeof(cfg.dns) - 1);
    strncpy(cfg.hostname, doc["hostname"] | "wt32-relays", sizeof(cfg.hostname) - 1);

    Network.updateConfig(cfg);
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleMqttConfigGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    const MqttConfig &cfg = Mqtt.getConfig();
    String out = "{\"enabled\":";
    out += cfg.enabled ? "true" : "false";
    out += ",\"host\":\"" + String(cfg.host) + "\"";
    out += ",\"port\":" + String(cfg.port);
    out += ",\"user\":\"" + String(cfg.user) + "\"";
    out += ",\"pass\":\"\"";
    out += ",\"client_id\":\"" + String(cfg.client_id) + "\"";
    out += ",\"base_topic\":\"" + String(cfg.base_topic) + "\"";
    out += ",\"ha_discovery\":";
    out += cfg.ha_discovery ? "true" : "false";
    out += ",\"use_ssl\":";
    out += cfg.use_ssl ? "true" : "false";
    out += ",\"ssl_mode\":";
    out += String(cfg.ssl_mode);
    out += "}";
    server.send(200, "application/json", out);
}

void WebServerManager::handleMqttConfigPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(512);
#endif
    deserializeJson(doc, server.arg("plain"));

    MqttConfig cfg = Mqtt.getConfig();
    cfg.enabled = doc["enabled"] | cfg.enabled;
    if (doc.containsKey("use_ssl")) cfg.use_ssl = doc["use_ssl"].as<bool>();
    if (doc.containsKey("ssl_mode")) cfg.ssl_mode = doc["ssl_mode"].as<uint8_t>();
    strncpy(cfg.host, doc["host"] | cfg.host, sizeof(cfg.host) - 1);
    cfg.port = doc["port"] | cfg.port;
    strncpy(cfg.user, doc["user"] | cfg.user, sizeof(cfg.user) - 1);
    const char *new_pass = doc["pass"] | "";
    if (strlen(new_pass) > 0) {
        strncpy(cfg.pass, new_pass, sizeof(cfg.pass) - 1);
    }
    strncpy(cfg.client_id, doc["client_id"] | cfg.client_id, sizeof(cfg.client_id) - 1);
    strncpy(cfg.base_topic, doc["base_topic"] | cfg.base_topic, sizeof(cfg.base_topic) - 1);
    cfg.ha_discovery = doc["ha_discovery"] | cfg.ha_discovery;

    Mqtt.updateConfig(cfg);
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleSecurityConfigGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    const SecurityConfig &cfg = Auth.getConfig();
    String out = "{\"username\":\"" + String(cfg.username) + "\"";
    out += ",\"session_timeout_s\":" + String(cfg.session_timeout_s);
    out += ",\"lockout_attempts\":" + String(cfg.lockout_attempts);
    out += ",\"lockout_time_s\":" + String(cfg.lockout_time_s) + "}";
    server.send(200, "application/json", out);
}

void WebServerManager::handleSecurityCredentialsPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(256);
#endif
    deserializeJson(doc, server.arg("plain"));

    const char *old_pass = doc["old_pass"] | "";
    const char *new_user = doc["new_user"] | "";
    const char *new_pass = doc["new_pass"] | "";

    if (Auth.changeCredentials(old_pass, new_user, new_pass)) {
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Incorrect current password\"}");
    }
}

void WebServerManager::handleSecurityPolicyPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(256);
#endif
    deserializeJson(doc, server.arg("plain"));

    SecurityConfig cfg = Auth.getConfig();
    cfg.session_timeout_s = doc["session_timeout_s"] | cfg.session_timeout_s;
    cfg.lockout_attempts = doc["lockout_attempts"] | cfg.lockout_attempts;
    cfg.lockout_time_s = doc["lockout_time_s"] | cfg.lockout_time_s;

    Auth.updateConfig(cfg);
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleLogsGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    int filter = -1;
    if (server.hasArg("subsys")) {
        filter = server.arg("subsys").toInt();
    }

    String out;
    EventLog.getAllJson(out, filter);
    server.send(200, "application/json", out);
}

void WebServerManager::handleLogsClear() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
    EventLog.clear();
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleSystemInfo() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    String out = "{";
    out += "\"firmware\":\"" + String(FIRMWARE_NAME) + "\",";
    out += "\"version\":\"" + String(FIRMWARE_VERSION) + "\",";
    out += "\"mac\":\"" + Network.getMac() + "\",";
    out += "\"ip\":\"" + Network.getIp() + "\",";
    out += "\"cpu_freq\":" + String(ESP.getCpuFreqMHz()) + ",";
    out += "\"temp\":" + String(temperatureRead(), 1) + ",";
    out += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
    out += "\"min_heap\":" + String(ESP.getMinFreeHeap()) + ",";
    out += "\"flash_size\":" + String(ESP.getFlashChipSize()) + ",";
    out += "\"out1_ok\":" + String(HW.isOut1Healthy() ? "true" : "false") + ",";
    out += "\"out2_ok\":" + String(HW.isOut2Healthy() ? "true" : "false") + ",";
    out += "\"in1_ok\":" + String(HW.isIn1Healthy() ? "true" : "false") + ",";
    out += "\"in2_ok\":" + String(HW.isIn2Healthy() ? "true" : "false") + "}";
    server.send(200, "application/json", out);
}

void WebServerManager::handleReboot() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
    server.send(200, "application/json", "{\"success\":true}");
    delay(200);
    ESP.restart();
}

void WebServerManager::handleFactoryReset() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
    server.send(200, "application/json", "{\"success\":true}");
    delay(200);
    Storage.factoryReset();
    ESP.restart();
}

void WebServerManager::handleUpdateUpload() {
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        if (!checkAuth()) {
            Update.abort();
            return;
        }
        EventLog.log(SUBSYS_SYSTEM, "OTA update start: %s", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
            EventLog.log(SUBSYS_SYSTEM, "OTA begin failed");
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        esp_task_wdt_reset();
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            EventLog.log(SUBSYS_SYSTEM, "OTA write failed");
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        esp_task_wdt_reset();
        if (Update.end(true)) {
            EventLog.log(SUBSYS_SYSTEM, "OTA update success (%u B)", upload.totalSize);
        } else {
            EventLog.log(SUBSYS_SYSTEM, "OTA finalize failed");
        }
    }
}

void WebServerManager::handleUpdateDone() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
    if (Update.hasError()) {
        server.send(500, "application/json", "{\"success\":false,\"error\":\"OTA flash error\"}");
    } else {
        server.send(200, "application/json", "{\"success\":true,\"reboot\":true}");
        delay(500);
        ESP.restart();
    }
}

void WebServerManager::handleBackup() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    String out = "{";
    out += "\"relays\":[";
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        if (i > 0) out += ",";
        const RelayConfig &rc = Relays.getConfig(i);
        out += "{\"name\":\"" + String(rc.name) + "\",\"enabled\":" + (rc.enabled?"true":"false") +
               ",\"restore_mode\":" + String((uint8_t)rc.restore_mode) +
               ",\"auto_off_sec\":" + String(rc.auto_off_sec) +
               ",\"on_delay_ms\":" + String(rc.on_delay_ms) +
               ",\"off_delay_ms\":" + String(rc.off_delay_ms) +
               ",\"interlock_group\":" + String(rc.interlock_group) + "}";
    }
    out += "],\"inputs\":[";
    for (uint8_t i = 0; i < NUM_INPUTS; ++i) {
        if (i > 0) out += ",";
        const InputConfig &ic = Inputs.getConfig(i);
        out += "{\"name\":\"" + String(ic.name) + "\",\"enabled\":" + (ic.enabled?"true":"false") +
               ",\"polarity\":" + String((uint8_t)ic.polarity) +
               ",\"mode\":" + String((uint8_t)ic.mode) +
               ",\"target_relay\":" + String(ic.target_relay) +
               ",\"short_action\":" + String((uint8_t)ic.short_action) +
               ",\"long_action\":" + String((uint8_t)ic.long_action) +
               ",\"double_action\":" + String((uint8_t)ic.double_action) +
               ",\"debounce_ms\":" + String(ic.debounce_ms) +
               ",\"long_press_ms\":" + String(ic.long_press_ms) +
               ",\"double_click_ms\":" + String(ic.double_click_ms) + "}";
    }
    out += "],\"schedules\":[";
    for (uint8_t i = 0; i < NUM_SCHEDULES; ++i) {
        if (i > 0) out += ",";
        const ScheduleItem &sc = Schedules.getSchedule(i);
        out += "{\"name\":\"" + String(sc.name) + "\",\"enabled\":" + (sc.enabled?"true":"false") +
               ",\"days_mask\":" + String(sc.days_mask) +
               ",\"hour\":" + String(sc.hour) +
               ",\"minute\":" + String(sc.minute) +
               ",\"action\":" + String((uint8_t)sc.action) +
               ",\"target_relay\":" + String(sc.target_relay) + "}";
    }
    out += "]";

    const MqttConfig &mc = Mqtt.getConfig();
    out += ",\"mqtt\":{\"enabled\":" + String(mc.enabled?"true":"false") +
           ",\"use_ssl\":" + String(mc.use_ssl?"true":"false") +
           ",\"ssl_mode\":" + String(mc.ssl_mode) +
           ",\"host\":\"" + String(mc.host) + "\"" +
           ",\"port\":" + String(mc.port) +
           ",\"user\":\"" + String(mc.user) + "\"" +
           ",\"client_id\":\"" + String(mc.client_id) + "\"" +
           ",\"base_topic\":\"" + String(mc.base_topic) + "\"" +
           ",\"ha_discovery\":" + String(mc.ha_discovery?"true":"false") + "}";

    const WifiConfig &wc = Network.getWifiConfig();
    out += ",\"wifi\":{\"sta_enabled\":" + String(wc.sta_enabled?"true":"false") +
           ",\"sta_ssid\":\"" + String(wc.sta_ssid) + "\"" +
           ",\"ap_fallback_enabled\":" + String(wc.ap_fallback_enabled?"true":"false") +
           ",\"ap_ssid\":\"" + String(wc.ap_ssid) + "\"}";

    const PingWatchdogConfig &wdc = PingWatchdog.getConfig();
    out += ",\"watchdog\":{\"enabled\":" + String(wdc.enabled?"true":"false") +
           ",\"host\":\"" + String(wdc.host) + "\"" +
           ",\"port\":" + String(wdc.port) +
           ",\"interval\":" + String(wdc.check_interval_s) +
           ",\"fails\":" + String(wdc.fail_threshold) +
           ",\"relay\":" + String(wdc.target_relay) +
           ",\"off_sec\":" + String(wdc.power_off_duration_s) +
           ",\"cooldown\":" + String(wdc.cooldown_s) + "}";

    out += "}";

    server.sendHeader("Content-Disposition", "attachment; filename=\"wt32_config_backup.json\"");
    server.send(200, "application/json", out);
}

void WebServerManager::handleRestore() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(4096);
#endif
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    if (doc.containsKey("relays")) {
        JsonArray rArr = doc["relays"];
        for (uint8_t i = 0; i < NUM_RELAYS && i < rArr.size(); ++i) {
            JsonObject rObj = rArr[i];
            RelayConfig rc = Relays.getConfig(i);
            if (rObj.containsKey("name")) strncpy(rc.name, rObj["name"] | rc.name, sizeof(rc.name) - 1);
            if (rObj.containsKey("enabled")) rc.enabled = rObj["enabled"].as<bool>();
            if (rObj.containsKey("restore_mode")) rc.restore_mode = (RelayRestoreMode)rObj["restore_mode"].as<uint8_t>();
            if (rObj.containsKey("auto_off_sec")) rc.auto_off_sec = rObj["auto_off_sec"].as<uint32_t>();
            if (rObj.containsKey("on_delay_ms")) rc.on_delay_ms = rObj["on_delay_ms"].as<uint32_t>();
            if (rObj.containsKey("off_delay_ms")) rc.off_delay_ms = rObj["off_delay_ms"].as<uint32_t>();
            if (rObj.containsKey("interlock_group")) rc.interlock_group = rObj["interlock_group"].as<uint8_t>();
            Relays.updateConfig(i, rc);
        }
    }

    if (doc.containsKey("inputs")) {
        JsonArray iArr = doc["inputs"];
        for (uint8_t i = 0; i < NUM_INPUTS && i < iArr.size(); ++i) {
            JsonObject iObj = iArr[i];
            InputConfig ic = Inputs.getConfig(i);
            if (iObj.containsKey("name")) strncpy(ic.name, iObj["name"] | ic.name, sizeof(ic.name) - 1);
            if (iObj.containsKey("enabled")) ic.enabled = iObj["enabled"].as<bool>();
            if (iObj.containsKey("polarity")) ic.polarity = (InputPolarity)iObj["polarity"].as<uint8_t>();
            if (iObj.containsKey("mode")) ic.mode = (InputMode)iObj["mode"].as<uint8_t>();
            if (iObj.containsKey("target_relay")) ic.target_relay = iObj["target_relay"].as<uint8_t>();
            if (iObj.containsKey("short_action")) ic.short_action = (InputAction)iObj["short_action"].as<uint8_t>();
            if (iObj.containsKey("long_action")) ic.long_action = (InputAction)iObj["long_action"].as<uint8_t>();
            if (iObj.containsKey("double_action")) ic.double_action = (InputAction)iObj["double_action"].as<uint8_t>();
            if (iObj.containsKey("debounce_ms")) ic.debounce_ms = iObj["debounce_ms"].as<uint16_t>();
            if (iObj.containsKey("long_press_ms")) ic.long_press_ms = iObj["long_press_ms"].as<uint16_t>();
            if (iObj.containsKey("double_click_ms")) ic.double_click_ms = iObj["double_click_ms"].as<uint16_t>();
            Inputs.updateConfig(i, ic);
        }
    }

    if (doc.containsKey("schedules")) {
        JsonArray sArr = doc["schedules"];
        for (uint8_t i = 0; i < NUM_SCHEDULES && i < sArr.size(); ++i) {
            JsonObject sObj = sArr[i];
            ScheduleItem sc = Schedules.getSchedule(i);
            if (sObj.containsKey("name")) strncpy(sc.name, sObj["name"] | sc.name, sizeof(sc.name) - 1);
            if (sObj.containsKey("enabled")) sc.enabled = sObj["enabled"].as<bool>();
            if (sObj.containsKey("days_mask")) sc.days_mask = sObj["days_mask"].as<uint8_t>();
            if (sObj.containsKey("hour")) sc.hour = sObj["hour"].as<uint8_t>();
            if (sObj.containsKey("minute")) sc.minute = sObj["minute"].as<uint8_t>();
            if (sObj.containsKey("action")) sc.action = (InputAction)sObj["action"].as<uint8_t>();
            if (sObj.containsKey("target_relay")) sc.target_relay = sObj["target_relay"].as<uint8_t>();
            Schedules.updateSchedule(i, sc);
        }
    }

    if (doc.containsKey("mqtt")) {
        JsonObject mObj = doc["mqtt"];
        MqttConfig mc = Mqtt.getConfig();
        if (mObj.containsKey("enabled")) mc.enabled = mObj["enabled"].as<bool>();
        if (mObj.containsKey("use_ssl")) mc.use_ssl = mObj["use_ssl"].as<bool>();
        if (mObj.containsKey("ssl_mode")) mc.ssl_mode = mObj["ssl_mode"].as<uint8_t>();
        if (mObj.containsKey("host")) strncpy(mc.host, mObj["host"] | mc.host, sizeof(mc.host) - 1);
        if (mObj.containsKey("port")) mc.port = mObj["port"].as<uint16_t>();
        if (mObj.containsKey("user")) strncpy(mc.user, mObj["user"] | mc.user, sizeof(mc.user) - 1);
        if (mObj.containsKey("client_id")) strncpy(mc.client_id, mObj["client_id"] | mc.client_id, sizeof(mc.client_id) - 1);
        if (mObj.containsKey("base_topic")) strncpy(mc.base_topic, mObj["base_topic"] | mc.base_topic, sizeof(mc.base_topic) - 1);
        if (mObj.containsKey("ha_discovery")) mc.ha_discovery = mObj["ha_discovery"].as<bool>();
        Mqtt.updateConfig(mc);
    }

    if (doc.containsKey("wifi")) {
        JsonObject wObj = doc["wifi"];
        WifiConfig wc = Network.getWifiConfig();
        if (wObj.containsKey("sta_enabled")) wc.sta_enabled = wObj["sta_enabled"].as<bool>();
        if (wObj.containsKey("sta_ssid")) strncpy(wc.sta_ssid, wObj["sta_ssid"] | wc.sta_ssid, sizeof(wc.sta_ssid) - 1);
        if (wObj.containsKey("ap_fallback_enabled")) wc.ap_fallback_enabled = wObj["ap_fallback_enabled"].as<bool>();
        if (wObj.containsKey("ap_ssid")) strncpy(wc.ap_ssid, wObj["ap_ssid"] | wc.ap_ssid, sizeof(wc.ap_ssid) - 1);
        Network.updateWifiConfig(wc);
    }

    if (doc.containsKey("watchdog")) {
        JsonObject wdObj = doc["watchdog"];
        PingWatchdogConfig wdc = PingWatchdog.getConfig();
        if (wdObj.containsKey("enabled")) wdc.enabled = wdObj["enabled"].as<bool>();
        if (wdObj.containsKey("host")) strncpy(wdc.host, wdObj["host"] | wdc.host, sizeof(wdc.host) - 1);
        if (wdObj.containsKey("port")) wdc.port = wdObj["port"].as<uint16_t>();
        if (wdObj.containsKey("interval")) wdc.check_interval_s = wdObj["interval"].as<uint16_t>();
        if (wdObj.containsKey("fails")) wdc.fail_threshold = wdObj["fails"].as<uint8_t>();
        if (wdObj.containsKey("relay")) wdc.target_relay = wdObj["relay"].as<uint8_t>();
        if (wdObj.containsKey("off_sec")) wdc.power_off_duration_s = wdObj["off_sec"].as<uint16_t>();
        if (wdObj.containsKey("cooldown")) wdc.cooldown_s = wdObj["cooldown"].as<uint16_t>();
        PingWatchdog.updateConfig(wdc);
    }

    broadcastState();
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleScheduleGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    String out = "[";
    for (uint8_t i = 0; i < NUM_SCHEDULES; ++i) {
        if (i > 0) out += ",";
        const ScheduleItem &sc = Schedules.getSchedule(i);
        out += "{\"index\":" + String(i) +
               ",\"name\":\"" + String(sc.name) + "\"" +
               ",\"enabled\":" + (sc.enabled ? "true" : "false") +
               ",\"days_mask\":" + String(sc.days_mask) +
               ",\"hour\":" + String(sc.hour) +
               ",\"minute\":" + String(sc.minute) +
               ",\"action\":" + String((uint8_t)sc.action) +
               ",\"target_relay\":" + String(sc.target_relay) +
               ",\"target_scene\":" + String(sc.target_scene) +
               ",\"trigger_type\":" + String((uint8_t)sc.trigger_type) +
               ",\"offset_minutes\":" + String(sc.offset_minutes) + "}";
    }
    out += "]";
    server.send(200, "application/json", out);
}

void WebServerManager::handleSchedulePost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(512);
#endif
    deserializeJson(doc, server.arg("plain"));

    int idx = doc["index"] | -1;
    if (idx >= 0 && idx < NUM_SCHEDULES) {
        ScheduleItem sc = Schedules.getSchedule(idx);
        if (doc.containsKey("name")) {
            const char *name = doc["name"] | sc.name;
            strncpy(sc.name, name, sizeof(sc.name) - 1);
            sc.name[sizeof(sc.name) - 1] = '\0';
        }
        if (doc.containsKey("enabled")) sc.enabled = doc["enabled"].as<bool>();
        if (doc.containsKey("days_mask")) sc.days_mask = doc["days_mask"].as<uint8_t>();
        if (doc.containsKey("hour")) sc.hour = doc["hour"].as<uint8_t>();
        if (doc.containsKey("minute")) sc.minute = doc["minute"].as<uint8_t>();
        if (doc.containsKey("action")) sc.action = (InputAction)doc["action"].as<uint8_t>();
        if (doc.containsKey("target_relay")) sc.target_relay = doc["target_relay"].as<uint8_t>();
        if (doc.containsKey("target_scene")) sc.target_scene = doc["target_scene"].as<uint8_t>();
        if (doc.containsKey("trigger_type")) sc.trigger_type = (ScheduleTriggerType)doc["trigger_type"].as<uint8_t>();
        if (doc.containsKey("offset_minutes")) sc.offset_minutes = doc["offset_minutes"].as<int8_t>();

        Schedules.updateSchedule(idx, sc);
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid index\"}");
    }
}

void WebServerManager::handleWifiConfigGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    const WifiConfig &wc = Network.getWifiConfig();
    const WifiRuntime &wr = Network.getWifiRuntime();
    String out = "{\"sta_enabled\":" + String(wc.sta_enabled ? "true" : "false") +
                 ",\"sta_ssid\":\"" + String(wc.sta_ssid) + "\"" +
                 ",\"sta_pass\":\"\"" +
                 ",\"sta_connected\":" + String(wr.sta_connected ? "true" : "false") +
                 ",\"sta_ip\":\"" + String(wr.sta_ip) + "\"" +
                 ",\"sta_rssi\":" + String(wr.sta_rssi) +
                 ",\"ap_fallback_enabled\":" + String(wc.ap_fallback_enabled ? "true" : "false") +
                 ",\"ap_ssid\":\"" + String(wc.ap_ssid) + "\"" +
                 ",\"ap_pass\":\"\"" +
                 ",\"ap_active\":" + String(wr.ap_active ? "true" : "false") +
                 ",\"ap_ip\":\"" + String(wr.ap_ip) + "\"" +
                 ",\"ap_clients\":" + String(wr.ap_clients) + "}";
    server.send(200, "application/json", out);
}

void WebServerManager::handleWifiConfigPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(512);
#endif
    deserializeJson(doc, server.arg("plain"));

    WifiConfig wc = Network.getWifiConfig();
    if (doc.containsKey("sta_enabled")) wc.sta_enabled = doc["sta_enabled"].as<bool>();
    if (doc.containsKey("sta_ssid")) {
        const char *s = doc["sta_ssid"] | wc.sta_ssid;
        strncpy(wc.sta_ssid, s, sizeof(wc.sta_ssid) - 1);
        wc.sta_ssid[sizeof(wc.sta_ssid) - 1] = '\0';
    }
    if (doc.containsKey("sta_pass")) {
        const char *p = doc["sta_pass"] | "";
        if (strlen(p) > 0) {
            strncpy(wc.sta_pass, p, sizeof(wc.sta_pass) - 1);
            wc.sta_pass[sizeof(wc.sta_pass) - 1] = '\0';
        }
    }
    if (doc.containsKey("ap_fallback_enabled")) wc.ap_fallback_enabled = doc["ap_fallback_enabled"].as<bool>();
    if (doc.containsKey("ap_ssid")) {
        const char *s = doc["ap_ssid"] | wc.ap_ssid;
        strncpy(wc.ap_ssid, s, sizeof(wc.ap_ssid) - 1);
        wc.ap_ssid[sizeof(wc.ap_ssid) - 1] = '\0';
    }
    if (doc.containsKey("ap_pass")) {
        const char *p = doc["ap_pass"] | "";
        if (strlen(p) >= 8 || strcmp(p, "open") == 0 || strcmp(p, "none") == 0) {
            strncpy(wc.ap_pass, p, sizeof(wc.ap_pass) - 1);
            wc.ap_pass[sizeof(wc.ap_pass) - 1] = '\0';
        } else if (strlen(p) > 0) {
            server.send(400, "application/json", "{\"error\":\"Wi-Fi AP password must be at least 8 characters\"}");
            return;
        }
    }

    Network.updateWifiConfig(wc);
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleWifiScanGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    int n = WiFi.scanNetworks();
    String out = "[";
    for (int i = 0; i < n; ++i) {
        if (i > 0) out += ",";
        out += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + ",\"enc\":" + String((int)WiFi.encryptionType(i)) + "}";
    }
    out += "]";
    WiFi.scanDelete();
    server.send(200, "application/json", out);
}

void WebServerManager::handleWatchdogConfigGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    const PingWatchdogConfig &wdc = PingWatchdog.getConfig();
    const PingWatchdogRuntime &wdr = PingWatchdog.getRuntime();
    String out = "{\"enabled\":" + String(wdc.enabled ? "true" : "false") +
                 ",\"host\":\"" + String(wdc.host) + "\"" +
                 ",\"port\":" + String(wdc.port) +
                 ",\"interval\":" + String(wdc.check_interval_s) +
                 ",\"fails\":" + String(wdc.fail_threshold) +
                 ",\"relay\":" + String(wdc.target_relay) +
                 ",\"off_sec\":" + String(wdc.power_off_duration_s) +
                 ",\"cooldown\":" + String(wdc.cooldown_s) +
                 ",\"current_fails\":" + String(wdr.current_fails) +
                 ",\"total_reboots\":" + String(wdr.total_reboots) +
                 ",\"is_power_cycling\":" + String(wdr.is_power_cycling ? "true" : "false") +
                 ",\"in_cooldown\":" + String(wdr.in_cooldown ? "true" : "false") +
                 ",\"last_result\":\"" + String(wdr.last_result) + "\"}";
    server.send(200, "application/json", out);
}

void WebServerManager::handleWatchdogConfigPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(512);
#endif
    deserializeJson(doc, server.arg("plain"));

    PingWatchdogConfig wdc = PingWatchdog.getConfig();
    if (doc.containsKey("enabled")) wdc.enabled = doc["enabled"].as<bool>();
    if (doc.containsKey("host")) {
        const char *h = doc["host"] | wdc.host;
        strncpy(wdc.host, h, sizeof(wdc.host) - 1);
        wdc.host[sizeof(wdc.host) - 1] = '\0';
    }
    if (doc.containsKey("port")) wdc.port = doc["port"].as<uint16_t>();
    if (doc.containsKey("interval")) wdc.check_interval_s = doc["interval"].as<uint16_t>();
    if (doc.containsKey("fails")) wdc.fail_threshold = doc["fails"].as<uint8_t>();
    if (doc.containsKey("relay")) wdc.target_relay = doc["relay"].as<uint8_t>();
    if (doc.containsKey("off_sec")) wdc.power_off_duration_s = doc["off_sec"].as<uint16_t>();
    if (doc.containsKey("cooldown")) wdc.cooldown_s = doc["cooldown"].as<uint16_t>();

    PingWatchdog.updateConfig(wdc);
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleWatchdogTestPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    bool ok = PingWatchdog.triggerTest();
    const PingWatchdogRuntime &wdr = PingWatchdog.getRuntime();
    String out = "{\"success\":" + String(ok ? "true" : "false") +
                 ",\"result\":\"" + String(wdr.last_result) + "\"}";
    server.send(200, "application/json", out);
}

void WebServerManager::broadcastState() {
    if (wsServer.connectedClients() == 0) return;
    String json;
    buildStatusJson(json);
    wsServer.broadcastTXT(json);
}

void WebServerManager::handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            break;
        case WStype_CONNECTED: {
            String json;
            buildStatusJson(json);
            wsServer.sendTXT(num, json);
            break;
        }
        case WStype_TEXT: {
#if ARDUINOJSON_VERSION_MAJOR >= 7
            JsonDocument doc;
#else
            DynamicJsonDocument doc(512);
#endif
            deserializeJson(doc, payload, length);

            const char *action_type = doc["type"] | "";
            const char *token = doc["token"] | "";

            if (!Auth.validateSession(String(token))) {
                wsServer.sendTXT(num, "{\"type\":\"auth_error\",\"error\":\"Invalid session\"}");
                return;
            }

            if (strcmp(action_type, "toggle") == 0) {
                int r_idx = doc["relay"] | -1;
                if (r_idx >= 0 && r_idx < NUM_RELAYS) {
                    Relays.toggleRelay(r_idx, SRC_WEB);
                }
            } else if (strcmp(action_type, "scene") == 0) {
                int sc_idx = doc["scene"] | -1;
                if (sc_idx >= 0 && sc_idx < NUM_SCENES) {
                    Scenes.activateScene(sc_idx);
                }
            } else if (strcmp(action_type, "ping") == 0) {
                wsServer.sendTXT(num, "{\"type\":\"pong\"}");
            }
            break;
        }
        default:
            break;
    }
}

void WebServerManager::update() {
    server.handleClient();
    wsServer.loop();

    unsigned long now = millis();
    if (now - last_broadcast_ms >= 1000UL) {
        last_broadcast_ms = now;
        bool has_active_timers = false;
        for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
            if (TimerMgr.isAutoOffActive(i)) {
                has_active_timers = true;
                break;
            }
        }
        if (has_active_timers) {
            broadcastState();
        }
    }
}

void WebServerManager::handleManifest() {
    String manifest = F("{\n"
                        "  \"name\": \"WT32 Relay Controller\",\n"
                        "  \"short_name\": \"Relays\",\n"
                        "  \"start_url\": \"/\",\n"
                        "  \"display\": \"standalone\",\n"
                        "  \"background_color\": \"#12161a\",\n"
                        "  \"theme_color\": \"#12161a\",\n"
                        "  \"icons\": [\n"
                        "    {\n"
                        "      \"src\": \"data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'%3E%3Ccircle cx='50' cy='50' r='45' fill='%232563eb'/%3E%3Cpath d='M35 50 L45 60 L65 40' stroke='white' stroke-width='8' fill='none' stroke-linecap='round' stroke-linejoin='round'/%3E%3C/svg%3E\",\n"
                        "      \"sizes\": \"192x192\",\n"
                        "      \"type\": \"image/svg+xml\"\n"
                        "    }\n"
                        "  ]\n"
                        "}");
    server.send(200, "application/manifest+json", manifest);
}

void WebServerManager::handleEnergyStatus() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    String output = "{";
    output += "\"total_power_w\":" + String(Relays.getTotalSystemPowerW(), 1) + ",";
    output += "\"total_kwh\":" + String(Relays.getTotalSystemKWh(), 3) + ",";
    output += "\"relays\":[";
    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        if (i > 0) output += ",";
        const RelayConfig &rc = Relays.getConfig(i);
        const RelayRuntime &rr = Relays.getRuntime(i);
        uint32_t sec = Relays.getRelayTotalOnSeconds(i);
        float kwh = Relays.getRelayKWh(i);
        uint32_t cycles = rr.energy.total_cycles;
        uint8_t health = (cycles >= 100000) ? 0 : (uint8_t)(100 - (cycles / 1000));

        output += "{\"idx\":" + String(i + 1);
        output += ",\"name\":\"" + String(rc.name) + "\"";
        output += ",\"power_w\":" + String(rc.rated_power_w);
        output += ",\"device_icon\":" + String(rc.device_icon);
        output += ",\"room\":\"" + String(rc.room) + "\"";
        output += ",\"state\":" + String(rr.state ? "true" : "false");
        output += ",\"cycles\":" + String(cycles);
        output += ",\"on_seconds\":" + String(sec);
        output += ",\"kwh\":" + String(kwh, 3);
        output += ",\"health\":" + String(health);
        output += "}";
    }
    output += "]}";
    server.send(200, "application/json", output);
}

void WebServerManager::handleEnergyReset() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
    if (!server.hasArg("plain")) { server.send(400, "application/json", "{\"error\":\"No body\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(256);
#endif
    deserializeJson(doc, server.arg("plain"));

    uint8_t r = doc["relay"] | 0;
    if (r == 0) {
        for (uint8_t i = 0; i < NUM_RELAYS; ++i) Relays.resetEnergyStats(i);
    } else if (r >= 1 && r <= NUM_RELAYS) {
        Relays.resetEnergyStats(r - 1);
    }
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleLogicConfigGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    String output = "{\"rules\":[";
    for (uint8_t i = 0; i < NUM_LOGIC_RULES; ++i) {
        if (i > 0) output += ",";
        const LogicRule &r = Logic.getRule(i);
        output += "{\"idx\":" + String(i + 1);
        output += ",\"enabled\":" + String(r.enabled ? "true" : "false");
        output += ",\"name\":\"" + String(r.name) + "\"";
        output += ",\"cond1_type\":" + String((uint8_t)r.cond1_type);
        output += ",\"cond1_target\":" + String(r.cond1_target);
        output += ",\"logic_op\":" + String((uint8_t)r.logic_op);
        output += ",\"cond2_type\":" + String((uint8_t)r.cond2_type);
        output += ",\"cond2_target\":" + String(r.cond2_target);
        output += ",\"action\":" + String((uint8_t)r.action);
        output += ",\"action_target\":" + String(r.action_target);
        output += "}";
    }
    output += "]}";
    server.send(200, "application/json", output);
}

void WebServerManager::handleLogicConfigPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
    if (!server.hasArg("plain")) { server.send(400, "application/json", "{\"error\":\"No body\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(512);
#endif
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) { server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}"); return; }

    uint8_t idx = (uint8_t)(doc["idx"] | 1) - 1;
    if (idx >= NUM_LOGIC_RULES) { server.send(400, "application/json", "{\"error\":\"Bad index\"}"); return; }

    LogicRule r = Logic.getRule(idx);
    r.enabled = doc["enabled"] | false;
    const char *n = doc["name"] | "";
    if (strlen(n) > 0) strncpy(r.name, n, sizeof(r.name) - 1);
    r.cond1_type = (LogicConditionType)(doc["cond1_type"] | 0);
    r.cond1_target = (uint8_t)(doc["cond1_target"] | 1);
    r.logic_op = (LogicOperator)(doc["logic_op"] | 0);
    r.cond2_type = (LogicConditionType)(doc["cond2_type"] | 0);
    r.cond2_target = (uint8_t)(doc["cond2_target"] | 1);
    r.action = (InputAction)(doc["action"] | 0);
    r.action_target = (uint8_t)(doc["action_target"] | 1);

    Logic.updateRule(idx, r);
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleTimeConfigGet() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }

    const TimeConfig &tc = Ntp.getConfig();
    char time_buf[32];
    Ntp.getTimeString(time_buf, sizeof(time_buf));

    String output = "{";
    output += "\"time\":\"" + String(time_buf) + "\",";
    output += "\"epoch\":" + String((unsigned long)time(nullptr)) + ",";
    output += "\"ntp_synced\":" + String(Ntp.isSynced() ? "true" : "false") + ",";
    output += "\"ntp\":\"" + String(tc.ntp_server) + "\",";
    output += "\"gmt\":" + String(tc.gmt_offset_s) + ",";
    output += "\"dst\":" + String(tc.dst_offset_s) + ",";
    output += "\"lat\":" + String(tc.latitude, 4) + ",";
    output += "\"lon\":" + String(tc.longitude, 4) + ",";
    output += "\"sunrise\":\"" + Astro.getSunriseFormatted() + "\",";
    output += "\"sunset\":\"" + Astro.getSunsetFormatted() + "\",";
    output += "\"solar_noon\":\"" + Astro.getSolarNoonFormatted() + "\",";
    output += "\"day_length\":\"" + Astro.getDayLengthFormatted() + "\",";
    output += "\"is_night\":" + String(Astro.isNight() ? "true" : "false");
    output += "}";
    server.send(200, "application/json", output);
}

void WebServerManager::handleTimeConfigPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
    if (!server.hasArg("plain")) { server.send(400, "application/json", "{\"error\":\"No body\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(384);
#endif
    deserializeJson(doc, server.arg("plain"));

    TimeConfig tc = Ntp.getConfig();
    const char *ntp = doc["ntp"] | "";
    if (strlen(ntp) > 0) strncpy(tc.ntp_server, ntp, sizeof(tc.ntp_server));
    if (doc.containsKey("gmt")) tc.gmt_offset_s = doc["gmt"];
    if (doc.containsKey("dst")) tc.dst_offset_s = doc["dst"];
    if (doc.containsKey("lat")) tc.latitude = doc["lat"];
    if (doc.containsKey("lon")) tc.longitude = doc["lon"];

    Ntp.updateConfig(tc);
    broadcastState();
    server.send(200, "application/json", "{\"success\":true}");
}

void WebServerManager::handleTimeSetPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
    if (!server.hasArg("plain")) { server.send(400, "application/json", "{\"error\":\"No body\"}"); return; }

#if ARDUINOJSON_VERSION_MAJOR >= 7
    JsonDocument doc;
#else
    DynamicJsonDocument doc(256);
#endif
    deserializeJson(doc, server.arg("plain"));

    time_t epoch = doc["epoch"] | 0;
    if (epoch > 100000) {
        int32_t gmt = doc.containsKey("gmt") ? doc["gmt"].as<int32_t>() : -1;
        Ntp.setManualTime(epoch, gmt);
        broadcastState();
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json", "{\"error\":\"Invalid epoch\"}");
    }
}

void WebServerManager::handleTimeSyncNtpPost() {
    if (!checkAuth()) { server.send(401, "application/json", "{\"error\":\"Unauthorized\"}"); return; }
    Ntp.forceSync();
    server.send(200, "application/json", "{\"success\":true}");
}

