#pragma once

#include <vector>

#include "GameClock.hpp"
#include "Camera.hpp"
#include "Entity.hpp"
#include "ShaderTypes.h"
#include "FrameData.h"

class Scene {
public:
    Camera camera;
    GameClock clock;
    
    Scene(size_t maxEntityCount, double timestep, Camera cam, GameClock clock);
    
    Entity& AddEntity(Entity&& entity);
    
    std::vector<Entity>& GetEntitiesOfType(uint32_t type);
    
    void Update();
    
    FrameData GetFrameData();
    
    double Timestep() const;
    
    size_t InstanceCount() const;
    size_t MaxInstanceCount() const;
protected:
    virtual void OnRender(double dt) = 0;
    virtual void OnUpdate() = 0;
private:
    std::vector<std::vector<Entity>> _entityGroups;
    std::vector<size_t> _groupSizes;
    
    size_t _instanceCount = 0;
    std::vector<Instance> _instances;
    
    size_t _maxInstanceCount;
    
    double _timestep;    
    double _nextTickTime;
    double _prevRenderTime;
};
