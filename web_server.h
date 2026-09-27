#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Update.h>
#include "config.h"

class WebServerManager {
public:
    WebServerManager();
    void begin();
    void update();

    void broadcastState();

    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);

private:
    WebServer server;
    WebSocketsServer wsServer;
    unsigned long last_broadcast_ms;

    bool checkAuth();
    String extractSessionToken();

    void setupRoutes();
    void handleRoot();
    void handleStatus();
    void handleSetup();
    void handleLogin();
    void handleLogout();
    void handleRelayToggle();
    void handleRelayAllOn();
    void handleRelayAllOff();
    void handleRelayConfigGet();
    void handleRelayConfigPost();
    void handleInterlockConfigGet();
    void handleInterlockConfigPost();
    void handleInputConfigGet();
    void handleInputConfigPost();
    void handleInputDefaultsPost();
    void handleInputDefaultsApplyAll();
    void handleInputDefaultsReset();
    void handleEmergencyReset();
    void handleSceneConfigGet();
    void handleSceneConfigPost();
    void handleSceneActivate();
    void handleNetworkConfigGet();
    void handleNetworkConfigPost();
    void handleMqttConfigGet();
    void handleMqttConfigPost();
    void handleSecurityConfigGet();
    void handleSecurityCredentialsPost();
    void handleSecurityPolicyPost();
    void handleLogsGet();
    void handleLogsClear();
    void handleSystemInfo();
    void handleReboot();
    void handleFactoryReset();
    void handleUpdateUpload();
    void handleUpdateDone();
    void handleBackup();
    void handleRestore();
    void handleScheduleGet();
    void handleSchedulePost();
    void handleWifiConfigGet();
    void handleWifiConfigPost();
    void handleWifiScanGet();
    void handleWatchdogConfigGet();
    void handleWatchdogConfigPost();
    void handleWatchdogTestPost();
    void handleManifest();
    void handleEnergyStatus();
    void handleEnergyReset();
    void handleLogicConfigGet();
    void handleLogicConfigPost();
    void handleTimeConfigGet();
    void handleTimeConfigPost();
    void handleTimeSetPost();
    void handleTimeSyncNtpPost();

    void buildStatusJson(String &output);

};

extern WebServerManager WebServerMgr;
