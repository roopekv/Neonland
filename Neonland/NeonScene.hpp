#pragma once

#include "Scene.hpp"
#include "NeonConstants.h"

class NeonScene : public Scene {
public:
    enum EntityGroup : uint32_t {
        PLAYER,
        ENEMY,
        CROSSHAIR,
        EntityGroupCount
    };
    
    float2 mouseDelta = {0, 0};
    float2 mousePos = {0, 0};
    float2 prevMousePos = {0, 0};
    
    float3 moveDir = {0, 0, 0};
    
    NeonScene(size_t maxEntityCount, double timestep, GameClock clock);
protected:
    void OnRender(double dt) override;
    void OnUpdate() override;
};
