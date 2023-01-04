#include "Neonland.h"
#include <vector>
#include <iostream>

#include "ThreadPool.hpp"
#include "GameClock.hpp"
#include "Enemies.hpp"
#include "Player.hpp"
#include "FrameData.h"
#include "Scene.hpp"

const double TIMESTEP = 1.0 / 30;

const size_t MAX_ENEMY_COUNT = 1'000'000;
const size_t MAX_PROJECTILE_COUNT = 0;
const size_t MAX_ENTITY_COUNT = MAX_ENEMY_COUNT + MAX_PROJECTILE_COUNT + 1;

namespace {

auto& threadPool = ThreadPool::GetInstance();
Scene scene(MAX_ENTITY_COUNT, TIMESTEP);

}

void OnStart() {
    scene.camera.position = {0, 0, -500};
    scene.camera.farPlane = abs(scene.camera.position.z) + 10;
    
    Player player;
    player.vel.x = 1;
    scene.AddEntity(player);
    
    int width = sqrt(MAX_ENEMY_COUNT), height = sqrt(MAX_ENEMY_COUNT);
    
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            Enemy enemy;
            enemy.SetPosition(vector_float3{static_cast<float>(-width/2 + x), static_cast<float>(-height/2 + y), 0.0f});
            enemy.angularVel = 30;
            scene.AddEntity(enemy);
        }
    }
}

FrameData OnRender(float aspectRatio) {
    scene.Update();
    
    return scene.GetFrameData(aspectRatio);
}


