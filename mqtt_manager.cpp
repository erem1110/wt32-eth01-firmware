#include "mqtt_manager.h"
#include "storage.h"
#include "network_manager.h"
#include "relay_subsystem.h"
#include "input_subsystem.h"
#include "emergency_subsystem.h"
#include "scene_subsystem.h"
#include "event_log.h"
#include "web_server.h"

static const char ISRG_ROOT_X1[] PROGMEM = 
"-----BEGIN CERTIFICATE-----\n"
"MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
"WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
"ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
"MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
"h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
"0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
"A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n"
"T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n"
"B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n"
"B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n"
"KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n"
"OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n"
"jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n"
"qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n"
"rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n"
"HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n"
"hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n"
"ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n"
"3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n"
"NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n"
"ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n"
"TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n"
"jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n"
"oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n"
"4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n"
"mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n"
"emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
"-----END CERTIFICATE-----\n";

static const char AMAZON_ROOT_CA_1[] PROGMEM = 
"-----BEGIN CERTIFICATE-----\n"
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
"rqXRfboQnoZsG4q5WTP468SQvvG5\n"
"-----END CERTIFICATE-----\n";

MqttManager Mqtt;

static void mqttCallback(char *topic, uint8_t *payload, unsigned int length) {
    Mqtt.onMessageReceived(topic, payload, length);
}

MqttManager::MqttManager() : client(ethClient), last_reconnect_attempt_ms(0) {}

void MqttManager::begin() {
    Storage.loadMqtt(config);
    client.setBufferSize(1024);
    client.setCallback(mqttCallback);
    client.setSocketTimeout(3);
    ethClient.setTimeout(3);
    secureClient.setTimeout(3);
}

bool MqttManager::isConnected() {
    return (config.enabled && client.connected());
}

void MqttManager::connectBroker() {
    if (!config.enabled || !Network.isConnected()) return;

    if (config.use_ssl) {
        secureClient.stop();
        secureClient.setTimeout(3);
        if (config.ssl_mode == 1) {
            secureClient.setCACert(ISRG_ROOT_X1);
        } else if (config.ssl_mode == 2) {
            secureClient.setCACert(AMAZON_ROOT_CA_1);
        } else {
            secureClient.setInsecure();
        }
        client.setClient(secureClient);
    } else {
        ethClient.stop();
        ethClient.setTimeout(3);
        client.setClient(ethClient);
    }

    client.setSocketTimeout(3);
    client.setServer(config.host, config.port);

    char lwt_topic[80];
    snprintf(lwt_topic, sizeof(lwt_topic), "%s/status", config.base_topic);

    bool ok = false;
    if (strlen(config.user) > 0) {
        ok = client.connect(config.client_id, config.user, config.pass, lwt_topic, 1, true, "offline");
    } else {
        ok = client.connect(config.client_id, lwt_topic, 1, true, "offline");
    }

    if (ok) {
        EventLog.log(SUBSYS_MQTT, "Connected to MQTT broker %s:%u", config.host, config.port);
        client.publish(lwt_topic, "online", true);

        char emg_topic[80];
        snprintf(emg_topic, sizeof(emg_topic), "%s/emergency", config.base_topic);
        client.publish(emg_topic, Emergency.isEmergencyActive() ? "ACTIVE" : "CLEAR", true);

        subscribeTopics();

        if (config.ha_discovery) {
            publishHomeAssistantDiscovery();
        }

        for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
            publishRelayState(i, Relays.getRelayState(i));
        }

        WebServerMgr.broadcastState();
    } else {
        EventLog.log(SUBSYS_MQTT, "MQTT connect failed (state %d)", client.state());
    }
}

void MqttManager::subscribeTopics() {
    char sub_topic[80];
    snprintf(sub_topic, sizeof(sub_topic), "%s/relay/+/set", config.base_topic);
    client.subscribe(sub_topic);

    snprintf(sub_topic, sizeof(sub_topic), "%s/scene/+/activate", config.base_topic);
    client.subscribe(sub_topic);

    snprintf(sub_topic, sizeof(sub_topic), "%s/emergency/set", config.base_topic);
    client.subscribe(sub_topic);
}

void MqttManager::publishHomeAssistantDiscovery() {
    char topic[128];
    char payload[512];

    for (uint8_t i = 0; i < NUM_RELAYS; ++i) {
        const RelayConfig &rc = Relays.getConfig(i);
        snprintf(topic, sizeof(topic), "homeassistant/switch/%s/relay_%u/config", config.client_id, i + 1);
        snprintf(payload, sizeof(payload),
                 "{\"name\":\"%s\",\"uniq_id\":\"%s_r%u\","
                 "\"stat_t\":\"%s/relay/%u/state\",\"cmd_t\":\"%s/relay/%u/set\","
                 "\"avty_t\":\"%s/status\",\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
                 "\"dev\":{\"ids\":[\"%s\"],\"name\":\"%s\",\"mf\":\"Custom\",\"mdl\":\"WT32-ETH01-16R\",\"sw\":\"1.0.0\"}}",
                 rc.name, config.client_id, i + 1,
                 config.base_topic, i + 1, config.base_topic, i + 1,
                 config.base_topic, config.client_id, FIRMWARE_NAME);
        client.publish(topic, payload, true);
    }

    for (uint8_t i = 0; i < NUM_INPUTS; ++i) {
        const InputConfig &ic = Inputs.getConfig(i);
        snprintf(topic, sizeof(topic), "homeassistant/binary_sensor/%s/input_%u/config", config.client_id, i + 1);
        snprintf(payload, sizeof(payload),
                 "{\"name\":\"%s\",\"uniq_id\":\"%s_in%u\","
                 "\"stat_t\":\"%s/input/%u/state\",\"avty_t\":\"%s/status\","
                 "\"pl_on\":\"ON\",\"pl_off\":\"OFF\","
                 "\"dev\":{\"ids\":[\"%s\"]}}",
                 ic.name, config.client_id, i + 1,
                 config.base_topic, i + 1, config.base_topic,
                 config.client_id);
        client.publish(topic, payload, true);
    }

    snprintf(topic, sizeof(topic), "homeassistant/binary_sensor/%s/emergency/config", config.client_id);
    snprintf(payload, sizeof(payload),
             "{\"name\":\"Emergency Status\",\"uniq_id\":\"%s_emg\","
             "\"stat_t\":\"%s/emergency\",\"avty_t\":\"%s/status\","
             "\"pl_on\":\"ACTIVE\",\"pl_off\":\"CLEAR\","
             "\"dev\":{\"ids\":[\"%s\"]}}",
             config.client_id, config.base_topic, config.base_topic, config.client_id);
    client.publish(topic, payload, true);
}

void MqttManager::publishRelayState(uint8_t index, bool state) {
    if (!isConnected() || index >= NUM_RELAYS) return;
    char topic[80];
    snprintf(topic, sizeof(topic), "%s/relay/%u/state", config.base_topic, index + 1);
    client.publish(topic, state ? "ON" : "OFF", true);
}

void MqttManager::publishInputState(uint8_t index, bool active) {
    if (!isConnected() || index >= NUM_INPUTS) return;
    char topic[80];
    snprintf(topic, sizeof(topic), "%s/input/%u/state", config.base_topic, index + 1);
    client.publish(topic, active ? "ON" : "OFF", false);
}

void MqttManager::publishInputEvent(uint8_t index, const char *event_type) {
    if (!isConnected() || index >= NUM_INPUTS) return;
    char topic[80];
    snprintf(topic, sizeof(topic), "%s/input/%u/event", config.base_topic, index + 1);
    client.publish(topic, event_type, false);
}

void MqttManager::publishEmergency(bool active) {
    if (!isConnected()) return;
    char topic[80];
    snprintf(topic, sizeof(topic), "%s/emergency", config.base_topic);
    client.publish(topic, active ? "ACTIVE" : "CLEAR", true);
}

void MqttManager::onMessageReceived(char *topic, uint8_t *payload, unsigned int length) {
    char msg[64];
    size_t copy_len = (length < sizeof(msg) - 1) ? length : (sizeof(msg) - 1);
    memcpy(msg, payload, copy_len);
    msg[copy_len] = '\0';

    char prefix[80];
    snprintf(prefix, sizeof(prefix), "%s/relay/", config.base_topic);
    if (strncmp(topic, prefix, strlen(prefix)) == 0) {
        char *p = topic + strlen(prefix);
        int r_num = atoi(p);
        if (r_num >= 1 && r_num <= NUM_RELAYS) {
            uint8_t idx = r_num - 1;
            if (strcasecmp(msg, "ON") == 0 || strcmp(msg, "1") == 0) {
                Relays.turnRelayOn(idx, SRC_MQTT);
            } else if (strcasecmp(msg, "OFF") == 0 || strcmp(msg, "0") == 0) {
                Relays.turnRelayOff(idx, SRC_MQTT);
            } else if (strcasecmp(msg, "TOGGLE") == 0) {
                Relays.toggleRelay(idx, SRC_MQTT);
            }
        }
        return;
    }

    snprintf(prefix, sizeof(prefix), "%s/scene/", config.base_topic);
    if (strncmp(topic, prefix, strlen(prefix)) == 0) {
        char *p = topic + strlen(prefix);
        int sc_num = atoi(p);
        if (sc_num >= 1 && sc_num <= NUM_SCENES) {
            Scenes.activateScene(sc_num - 1);
        }
        return;
    }

    snprintf(prefix, sizeof(prefix), "%s/emergency/set", config.base_topic);
    if (strcmp(topic, prefix) == 0) {
        if (strcasecmp(msg, "RESET") == 0 || strcasecmp(msg, "CLEAR") == 0 || strcmp(msg, "0") == 0) {
            Emergency.resetEmergency(true);
        } else if (strcasecmp(msg, "TRIGGER") == 0 || strcasecmp(msg, "ACTIVE") == 0 || strcmp(msg, "1") == 0) {
            Emergency.triggerEmergency("MQTT");
        }
    }
}

void MqttManager::updateConfig(const MqttConfig &cfg) {
    if (client.connected()) {
        char lwt_topic[80];
        snprintf(lwt_topic, sizeof(lwt_topic), "%s/status", config.base_topic);
        client.publish(lwt_topic, "offline", true);
        client.disconnect();
    }
    config = cfg;
    Storage.saveMqtt(config);
    EventLog.log(SUBSYS_MQTT, "MQTT settings updated");
    WebServerMgr.broadcastState();
}

void MqttManager::update() {
    if (!config.enabled) return;

    if (!client.connected()) {
        unsigned long now = millis();
        if (now - last_reconnect_attempt_ms > 5000UL) {
            last_reconnect_attempt_ms = now;
            connectBroker();
        }
    } else {
        client.loop();
    }
}
