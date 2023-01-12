#include "NeonScene.hpp"

#include <iostream>
#include <random>

NeonScene::NeonScene(size_t maxEntityCount, double timestep, GameClock clock)
: Scene(maxEntityCount, timestep, Camera(), clock) {
    AddEntity(Entity(PLAYER));
    AddEntity(Entity(CROSSHAIR));
    
    camera.SetPosition({0, 0, -20});
    camera.SetFarClipPlane(200);
    
    int columns = sqrt(MAX_ENEMY_COUNT), rows = sqrt(MAX_ENEMY_COUNT);
    
    float gap = 0.5f;
    
    float width = (rows - 1) * (1 + gap);
    float height = (columns - 1) * (1 + gap);
    
    std::random_device device;
    std::default_random_engine engine(device());
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    
    for (int x = 0; x < columns; x++) {
        for (int y = 0; y < rows; y++) {
            float3 pos = {x * (1 + gap) - width / 2, y * (1 + gap) - height / 2};
            auto& enemy = AddEntity(Entity(ENEMY, pos));
            enemy.angularVelocity = 30 * distribution(engine);
        }
    }
}

void NeonScene::OnUpdate() {
    auto& player = GetEntitiesOfType(PLAYER)[0];
    player.velocity = moveDir * 10;
    
    moveDir = {0, 0, 0};
}

void NeonScene::OnRender(double dt) {
    auto& crosshair = GetEntitiesOfType(CROSSHAIR)[0];
    
    crosshair.SetPosition(camera.ScreenPointToWorld(mousePos, crosshair.GetPosition().z));
    mouseDelta = {0, 0};
}
