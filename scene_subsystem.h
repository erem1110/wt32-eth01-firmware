#pragma once

#include <Arduino.h>
#include "config.h"
#include "types.h"

class SceneSubsystem {
public:
    SceneSubsystem();
    void begin();

    void activateScene(uint8_t index);
    const SceneConfig& getConfig(uint8_t index) const { return scenes[index]; }
    void updateConfig(uint8_t index, const SceneConfig &cfg);

private:
    SceneConfig scenes[NUM_SCENES];
};

extern SceneSubsystem Scenes;
