#pragma once

#include <Arduino.h>
#include <ETH.h>
#include <WiFi.h>
#include "types.h"

class NetworkManager {
public:
    NetworkManager();
    void begin();
    void update();

    bool isConnected() const { return eth_got_ip || wifi_sta_got_ip; }
    bool isEthConnected() const { return eth_got_ip; }
    bool isWifiStaConnected() const { return wifi_sta_got_ip; }
    bool isWifiApActive() const { return wifi_ap_active; }

    String getIp() const;
    String getMac() const;
    String getHostname() const;
    uint32_t getLinkSpeed() const;
    bool isFullDuplex() const;

    const NetworkConfig& getConfig() const { return config; }
    void updateConfig(const NetworkConfig &cfg);

    const WifiConfig& getWifiConfig() const { return wifi_config; }
    const WifiRuntime& getWifiRuntime() const { return wifi_runtime; }
    void updateWifiConfig(const WifiConfig &wcfg);

    void onWifiEvent(WiFiEvent_t event);

    void startWifiAp();
    void stopWifiAp();
    void startWifiSta();
    void stopWifiSta();

private:
    NetworkConfig config;
    WifiConfig wifi_config;
    WifiRuntime wifi_runtime;

    bool eth_connected;
    bool eth_got_ip;
    bool wifi_sta_connected;
    bool wifi_sta_got_ip;
    bool wifi_ap_active;

    unsigned long last_sta_reconnect_ms;
    unsigned long eth_down_timer_ms;

    void evaluateFailover();
};

extern NetworkManager Network;
