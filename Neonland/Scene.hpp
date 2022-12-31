#pragma once

#include <vector>
#include <simd/simd.h>

#include "GameClock.hpp"
#include "Enemies.hpp"
#include "Player.hpp"
#include "ShaderTypes.h"

class Scene {
public:
    std::vector<Enemy> enemies;
    Player player;
    vector_float3 camPos;
    
    const double timestep;
    
    Instance * instanceBuffer;
    const size_t maxEntityCount;
    
    Scene(size_t maxEntityCount, double timestep);
    ~Scene();
    
//    Scene(const Scene& other);
//    Scene& operator=(const Scene& rhs);
    
    void Update();
    size_t InstanceCount() const;
private:
    GameClock gameClock;
    double nextTickTime;
};
