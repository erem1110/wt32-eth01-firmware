#include "scene_subsystem.h"
#include "storage.h"
#include "relay_subsystem.h"
#include "event_log.h"
#include "web_server.h"

SceneSubsystem Scenes;

SceneSubsystem::SceneSubsystem() {}

void SceneSubsystem::begin() {
    for (uint8_t i = 0; i < NUM_SCENES; ++i) {
        Storage.loadScene(i, scenes[i]);
    }
}

void SceneSubsystem::activateScene(uint8_t index) {
    if (index >= NUM_SCENES) return;

    EventLog.log(SUBSYS_SYSTEM, "Activating %s", scenes[index].name);

    for (uint8_t r = 0; r < NUM_RELAYS; ++r) {
        if (scenes[index].actions[r] == SCENE_RELAY_OFF) {
            Relays.turnRelayOff(r, SRC_SCENE);
        }
    }

    for (uint8_t r = 0; r < NUM_RELAYS; ++r) {
        if (scenes[index].actions[r] == SCENE_RELAY_ON) {
            Relays.turnRelayOn(r, SRC_SCENE);
        }
    }

    WebServerMgr.broadcastState();
}

void SceneSubsystem::updateConfig(uint8_t index, const SceneConfig &cfg) {
    if (index >= NUM_SCENES) return;
    scenes[index] = cfg;
    Storage.saveScene(index, cfg);
    EventLog.log(SUBSYS_SYSTEM, "Scene %u config updated", index + 1);
}
