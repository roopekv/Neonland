#include "Scene.hpp"

Scene::Scene(size_t maxEntityCount, double timestep)
: maxEntityCount{maxEntityCount}
, timestep{timestep}
, instanceBuffer{new Instance[maxEntityCount]}
, camPos{0, 0, 0}
, nextTickTime{gameClock.Time()} {}

Scene::~Scene() {
    delete[] instanceBuffer;
}

void Scene::Update() {
    auto time = gameClock.Time();
    
    while (time >= nextTickTime) {
        for (auto& enemy : enemies) {
            enemy.Update(timestep);
        }
        player.Update(timestep);
        nextTickTime += timestep;
    }
    
    double timeSinceUpdate = timestep - (nextTickTime - time);
    
    for (size_t i = 0; i < enemies.size(); i++) {
        auto& enemy = enemies[i];
        instanceBuffer[i] = enemy.Instance(timeSinceUpdate);
    }
    
    instanceBuffer[enemies.size()] = player.Instance(timeSinceUpdate);
}

size_t Scene::InstanceCount() const {
    return enemies.size() + 1;
}
