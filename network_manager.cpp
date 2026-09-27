#include "network_manager.h"
#include "storage.h"
#include "event_log.h"
#include "web_server.h"

NetworkManager Network;

static void wifiEventCallback(WiFiEvent_t event) {
    Network.onWifiEvent(event);
}

NetworkManager::NetworkManager() :
    eth_connected(false), eth_got_ip(false),
    wifi_sta_connected(false), wifi_sta_got_ip(false), wifi_ap_active(false),
    last_sta_reconnect_ms(0), eth_down_timer_ms(0) {
    memset(&wifi_runtime, 0, sizeof(wifi_runtime));
}

void NetworkManager::begin() {
    Storage.loadNetwork(config);
    Storage.loadWifi(wifi_config);

    WiFi.onEvent(wifiEventCallback);

    pinMode(ETH_PHY_POWER, OUTPUT);
    digitalWrite(ETH_PHY_POWER, HIGH);
    delay(500);

    if (!config.dhcp) {
        IPAddress ip, gw, mask, dns;
        if (ip.fromString(config.ip) && gw.fromString(config.gateway) &&
            mask.fromString(config.subnet) && dns.fromString(config.dns)) {
            ETH.config(ip, gw, mask, dns);
        }
    }

    ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
    ETH.setHostname(config.hostname);
    eth_down_timer_ms = millis();

    // Start Wi-Fi AP immediately so phone can ALWAYS connect to WT32-Relay-Setup
    if (wifi_config.ap_fallback_enabled) {
        startWifiAp();
    }
}

void NetworkManager::onWifiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            ETH.setHostname(config.hostname);
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            eth_connected = true;
            EventLog.log(SUBSYS_NETWORK, "Ethernet cable connected (%d Mbps, %s)",
                         ETH.linkSpeed(), ETH.fullDuplex() ? "Full Duplex" : "Half Duplex");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            eth_got_ip = true;
            eth_down_timer_ms = 0;
            EventLog.log(SUBSYS_NETWORK, "Ethernet IP obtained: %s", ETH.localIP().toString().c_str());
            if (wifi_sta_connected) {
                stopWifiSta();
            }
            if (wifi_ap_active) {
                stopWifiAp();
            }
            WebServerMgr.broadcastState();
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            eth_connected = false;
            eth_got_ip = false;
            eth_down_timer_ms = millis();
            EventLog.log(SUBSYS_NETWORK, "Ethernet cable disconnected");
            evaluateFailover();
            WebServerMgr.broadcastState();
            break;
        case ARDUINO_EVENT_ETH_STOP:
            eth_connected = false;
            eth_got_ip = false;
            break;

        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            wifi_sta_connected = true;
            wifi_runtime.sta_connected = true;
            EventLog.log(SUBSYS_NETWORK, "Wi-Fi STA connected to %s", wifi_config.sta_ssid);
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            wifi_sta_got_ip = true;
            strncpy(wifi_runtime.sta_ip, WiFi.localIP().toString().c_str(), sizeof(wifi_runtime.sta_ip));
            wifi_runtime.sta_rssi = WiFi.RSSI();
            EventLog.log(SUBSYS_NETWORK, "Wi-Fi STA IP obtained: %s", wifi_runtime.sta_ip);
            if (wifi_ap_active) {
                stopWifiAp();
            }
            WebServerMgr.broadcastState();
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            wifi_sta_connected = false;
            wifi_sta_got_ip = false;
            wifi_runtime.sta_connected = false;
            wifi_runtime.sta_ip[0] = '\0';
            wifi_runtime.sta_rssi = 0;
            EventLog.log(SUBSYS_NETWORK, "Wi-Fi STA disconnected");
            evaluateFailover();
            WebServerMgr.broadcastState();
            break;

        case ARDUINO_EVENT_WIFI_AP_START:
            wifi_ap_active = true;
            wifi_runtime.ap_active = true;
            strncpy(wifi_runtime.ap_ip, WiFi.softAPIP().toString().c_str(), sizeof(wifi_runtime.ap_ip));
            EventLog.log(SUBSYS_NETWORK, "Wi-Fi AP active: %s (%s)", wifi_config.ap_ssid, wifi_runtime.ap_ip);
            WebServerMgr.broadcastState();
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            wifi_ap_active = false;
            wifi_runtime.ap_active = false;
            wifi_runtime.ap_ip[0] = '\0';
            EventLog.log(SUBSYS_NETWORK, "Wi-Fi AP stopped");
            WebServerMgr.broadcastState();
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            wifi_runtime.ap_clients++;
            EventLog.log(SUBSYS_NETWORK, "Client connected to Wi-Fi AP (total: %u)", wifi_runtime.ap_clients);
            WebServerMgr.broadcastState();
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            if (wifi_runtime.ap_clients > 0) wifi_runtime.ap_clients--;
            EventLog.log(SUBSYS_NETWORK, "Client disconnected from Wi-Fi AP (total: %u)", wifi_runtime.ap_clients);
            WebServerMgr.broadcastState();
            break;

        default:
            break;
    }
}

void NetworkManager::evaluateFailover() {
    if (eth_got_ip) return;

    if (wifi_config.sta_enabled && strlen(wifi_config.sta_ssid) > 0 && !wifi_sta_got_ip) {
        startWifiSta();
    }

    if (wifi_config.ap_fallback_enabled && !wifi_sta_got_ip && !wifi_ap_active) {
        startWifiAp();
    }
}

void NetworkManager::startWifiAp() {
    if (wifi_ap_active) return;
    WiFiMode_t current_mode = WiFi.getMode();
    if (current_mode == WIFI_STA || current_mode == WIFI_AP_STA) {
        WiFi.mode(WIFI_AP_STA);
    } else {
        WiFi.mode(WIFI_AP);
    }
    WiFi.setSleep(false);
    const char *pass = (strlen(wifi_config.ap_pass) >= 8) ? wifi_config.ap_pass : "12345678";
    if (strcmp(wifi_config.ap_pass, "none") == 0 || strcmp(wifi_config.ap_pass, "open") == 0) {
        pass = NULL;
    }
    WiFi.softAP(wifi_config.ap_ssid, pass, 1, 0, 4);
    wifi_ap_active = true;
    wifi_runtime.ap_active = true;
    strncpy(wifi_runtime.ap_ip, WiFi.softAPIP().toString().c_str(), sizeof(wifi_runtime.ap_ip));
    EventLog.log(SUBSYS_NETWORK, "Wi-Fi AP active: %s (IP: %s, pass: %s)",
                 wifi_config.ap_ssid, wifi_runtime.ap_ip, pass ? pass : "[OPEN]");
}

void NetworkManager::stopWifiAp() {
    if (!wifi_ap_active) return;
    WiFi.softAPdisconnect(true);
    wifi_ap_active = false;
    wifi_runtime.ap_active = false;
    if (!wifi_sta_connected) {
        WiFi.mode(WIFI_OFF);
    } else {
        WiFi.mode(WIFI_STA);
    }
}

void NetworkManager::startWifiSta() {
    if (strlen(wifi_config.sta_ssid) == 0) return;
    WiFiMode_t current_mode = WiFi.getMode();
    if (current_mode == WIFI_AP || current_mode == WIFI_AP_STA) {
        WiFi.mode(WIFI_AP_STA);
    } else {
        WiFi.mode(WIFI_STA);
    }
    WiFi.begin(wifi_config.sta_ssid, wifi_config.sta_pass);
    last_sta_reconnect_ms = millis();
}

void NetworkManager::stopWifiSta() {
    WiFi.disconnect(true);
    wifi_sta_connected = false;
    wifi_sta_got_ip = false;
    wifi_runtime.sta_connected = false;
    if (wifi_ap_active) {
        WiFi.mode(WIFI_AP);
    } else {
        WiFi.mode(WIFI_OFF);
    }
}

String NetworkManager::getIp() const {
    if (eth_got_ip) {
        return ETH.localIP().toString();
    }
    if (wifi_sta_got_ip) {
        return WiFi.localIP().toString();
    }
    if (wifi_ap_active) {
        return WiFi.softAPIP().toString();
    }
    return String("0.0.0.0");
}

String NetworkManager::getMac() const {
    return ETH.macAddress();
}

String NetworkManager::getHostname() const {
    return String(config.hostname);
}

uint32_t NetworkManager::getLinkSpeed() const {
    return ETH.linkSpeed();
}

bool NetworkManager::isFullDuplex() const {
    return ETH.fullDuplex();
}

void NetworkManager::updateConfig(const NetworkConfig &cfg) {
    config = cfg;
    Storage.saveNetwork(config);
    EventLog.log(SUBSYS_NETWORK, "Network config saved (Reboot recommended)");
}

void NetworkManager::updateWifiConfig(const WifiConfig &wcfg) {
    wifi_config = wcfg;
    Storage.saveWifi(wifi_config);
    EventLog.log(SUBSYS_NETWORK, "Wi-Fi settings saved");
    if (wifi_ap_active) {
        stopWifiAp();
        startWifiAp();
    } else {
        evaluateFailover();
    }
    WebServerMgr.broadcastState();
}

void NetworkManager::update() {
    // If Ethernet has no IP and AP is not active, ensure fallback AP is active
    if (!eth_got_ip && !wifi_sta_got_ip && !wifi_ap_active && wifi_config.ap_fallback_enabled) {
        if (eth_down_timer_ms == 0) eth_down_timer_ms = millis();
        if (millis() - eth_down_timer_ms > 3000UL) {
            evaluateFailover();
        }
    }

    // Periodic check for Wi-Fi STA reconnect if STA enabled but disconnected while Ethernet is also down
    if (!eth_got_ip && wifi_config.sta_enabled && !wifi_sta_got_ip) {
        if (millis() - last_sta_reconnect_ms > 20000UL) {
            last_sta_reconnect_ms = millis();
            startWifiSta();
        }
    }
}
