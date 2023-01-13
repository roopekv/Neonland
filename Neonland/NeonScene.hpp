#pragma once

#include "Scene.hpp"
#include "GameClock.hpp"
#include "NeonConstants.h"

#include "Transform.hpp"
#include "Physics.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"

#include "FrameData.h"

class NeonScene {
public:
    GameClock clock;
    
    float2 mouseDelta = {0, 0};
    float2 mousePos = {0, 0};
    float2 prevMousePos = {0, 0};
    
    float3 moveDir = {0, 0, 0};
    
    Entity playerEntity;
    Entity camEntity;
    Entity crosshairEntity;
    
    NeonScene(size_t maxInstanceCount, double timestep, GameClock clock);
    
    void Update(float aspectRatio);
    
    void OnUpdate();
    void OnRender(double dt);
    
    FrameData GetFrameData();
    
    double Timestep() const;
    
    size_t MaxInstanceCount() const;
private:
    Scene _scene;
    
    std::vector<Instance> _instances;
    std::vector<size_t> _groupSizes;
    
    size_t _maxInstanceCount;
    
    double _timestep;
    double _nextTickTime;
    double _prevRenderTime;
};
