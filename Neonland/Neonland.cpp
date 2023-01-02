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
    Player player;
    player.vel.x = 1;
    scene.AddEntity(player);
    
    Enemy enemy;
    enemy.vel = {0, 0, 1};
    enemy.angularVel = 30;
    scene.AddEntity(enemy);
    
    scene.camPos = {0, 0, -20};
    
//    Enemy enemy;
//    scene.enemies.push_back(enemy);
}

FrameData OnRender(float aspectRatio) {
    scene.Update();
    Update();
    
    return scene.GetFrameData(aspectRatio);
}


