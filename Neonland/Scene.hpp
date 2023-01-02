#pragma once

#include <vector>
#include <simd/simd.h>

#include "GameClock.hpp"
#include "Enemies.hpp"
#include "Player.hpp"
#include "ShaderTypes.h"
#include "FrameData.h"

class Scene {
public:
    vector_float3 camPos;
    
    Scene(size_t maxEntityCount, double timestep);
    ~Scene();
    
    Scene(const Scene& other);
    Scene& operator=(const Scene& rhs);
    
    Entity& AddEntity(Entity entity);
    
    void Update();
    
    FrameData GetFrameData(float aspectRatio);
    
    size_t InstanceCount() const;
    size_t MaxInstanceCount() const;
private:
    std::vector<std::vector<Entity>> entityGroups;
    std::vector<size_t> _groupSizes;
    
    size_t _maxInstanceCount;
    Instance* _instanceBuffer;
    
    size_t _instanceCount;
    
    const double _timestep;
    
    GameClock _gameClock;
    double _nextTickTime;
};
