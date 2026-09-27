#pragma once

#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "types.h"

class MqttManager {
public:
    MqttManager();
    void begin();
    void update();

    bool isConnected();
    bool isEnabled() const { return config.enabled; }
    const MqttConfig& getConfig() const { return config; }
    void updateConfig(const MqttConfig &cfg);

    void publishRelayState(uint8_t index, bool state);
    void publishInputState(uint8_t index, bool active);
    void publishInputEvent(uint8_t index, const char *event_type);
    void publishEmergency(bool active);

    void onMessageReceived(char *topic, uint8_t *payload, unsigned int length);

private:
    MqttConfig config;
    WiFiClient ethClient;
    WiFiClientSecure secureClient;
    PubSubClient client;
    unsigned long last_reconnect_attempt_ms;

    void connectBroker();
    void subscribeTopics();
    void publishHomeAssistantDiscovery();
};

extern MqttManager Mqtt;
