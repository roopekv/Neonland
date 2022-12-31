#include "Neonland.h"
#include <vector>
#include <iostream>

#include "GameClock.hpp"
#include "Enemies.hpp"
#include "Player.hpp"
#include "FrameData.h"
#include "Scene.hpp"

namespace {

Scene scene(MAX_ENTITY_COUNT, TIMESTEP);

void Update() {
    // GAMEPLAY HERE
}

}

void OnStart() {
    scene.player.vel.x = 1;
    scene.camPos = {0, 0, 50};
}

FrameData OnRender(float aspectRatio) {
    Update();
    scene.Update();
    
    FrameData frameData;
    frameData.globalUniforms.viewMatrix = TranslationMatrix(-scene.camPos);
    frameData.globalUniforms.projMatrix = ProjectionMatrix(60, aspectRatio, 0.1, 100);
    frameData.instanceCount = scene.enemies.size() + 1;
    frameData.instances = scene.instanceBuffer;
    return frameData;
}


