#include "Neonland.h"
#include <vector>
#include <iostream>
#include <random>

#include "Scene.hpp"

const double TIMESTEP = 1.0 / 30;

const size_t MAX_ENEMY_COUNT = 100;
const size_t MAX_PROJECTILE_COUNT = 0;
const size_t MAX_ENTITY_COUNT = MAX_ENEMY_COUNT + MAX_PROJECTILE_COUNT + 1;

namespace {

Scene scene(MAX_ENTITY_COUNT, TIMESTEP, Camera());

vector_float2 mouseDelta = {0, 0};
vector_float2 mousePos = {0, 0};
vector_float2 prevMousePos = {0, 0};

vector_float3 moveDir = {0, 0, 0};
}

void Neonland_Start() {
    auto& player = scene.AddEntity(Entity(0));
    auto& crosshair = scene.AddEntity(Entity(0));
    
    scene.camera.SetPosition({0, 0, -20});
    scene.camera.SetFarClipPlane(200);
//    scene.camera.SetFarClipPlane(abs(scene.camera.position.z) + 100);
    
    int columns = sqrt(MAX_ENEMY_COUNT), rows = sqrt(MAX_ENEMY_COUNT);
    
    float gap = 0.5f;
    
    float width = (rows - 1) * (1 + gap);
    float height = (columns - 1) * (1 + gap);
    
    std::random_device device;
    std::default_random_engine engine(device());
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    
    for (int x = 0; x < columns; x++) {
        for (int y = 0; y < rows; y++) {
            vector_float3 pos = {x * (1 + gap) - width / 2, y * (1 + gap) - height / 2};
            auto& enemy = scene.AddEntity(Entity(1, pos));
            enemy.angularVelocity = 30 * distribution(engine);
        }
    }
}

FrameData Neonland_Render(float aspectRatio) {
    if (aspectRatio != scene.camera.GetAspectRatio()) {
        scene.camera.SetAspectRatio(aspectRatio);
    }
    
    auto& player = scene.GetEntityGroup(0)[0];
    auto& crosshair = scene.GetEntityGroup(0)[1];
    
    player.velocity = moveDir;
    
    crosshair.SetPosition(scene.camera.ScreenPointToWorld(mousePos, 0));
    mouseDelta = {0, 0};
    
    scene.Update();
    
    return scene.GetFrameData();
}

void Neonland_UpdateCursorPosition(vector_float2 newPos) {
    prevMousePos = mousePos;
    
    mousePos.x = std::clamp(newPos.x, -1.0f, 1.0f);
    mousePos.y = std::clamp(newPos.y, -1.0f, 1.0f);
    mouseDelta += mousePos - prevMousePos;
}

void Neonland_UpdateMoveDirection(vector_float2 newDir) {
    moveDir.x = std::clamp(newDir.x, -1.0f, 1.0f);
    moveDir.y = std::clamp(newDir.y, -1.0f, 1.0f);
}

