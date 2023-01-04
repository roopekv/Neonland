#pragma once

#include <vector>
#include <simd/simd.h>

#include "GameClock.hpp"
#include "Camera.hpp"
#include "Enemies.hpp"
#include "Player.hpp"
#include "ShaderTypes.h"
#include "FrameData.h"

class Scene {
public:
    Camera camera;
    
    Scene(size_t maxEntityCount, double timestep, Camera cam = Camera());
    
    Entity& AddEntity(Entity entity);
    Entity& AddEntity();
    
    void Update();
    
    FrameData GetFrameData(float aspectRatio);
    
    size_t InstanceCount() const;
    size_t MaxInstanceCount() const;
private:
    std::vector<std::vector<Entity>> _entityGroups;
    std::vector<size_t> _groupSizes;
    
    GameClock _gameClock;
    
    size_t _instanceCount = 0;
    std::vector<Instance> _instances;
    
    size_t _maxInstanceCount;
    
    double _timestep;    
    double _nextTickTime;
};
