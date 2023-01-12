#pragma once

#include "MathUtils.hpp"
#include "ShaderTypes.h"

class Entity {
public:
    float3 velocity;
    float3 angularVelocity;
    
    float3 scale;
    
    const uint32_t type;
    
    Entity(uint32_t type, float3 pos = {0, 0, 0}, float3 rot = {0, 0, 0}, float3 scale = {1, 1, 1});
    
    void Update(double timestep);
    
    void SetPosition(float3 pos);
    void SetRotation(float3 rot);
    
    const float3& GetPosition() const;
    const float3& GetRotation() const;
    
    float4x4 GetTransform(double timeSinceUpdate) const;
    Instance GetInstance(double timeSinceUpdate) const;
protected:
    float3 _position;
    float3 _rotation;
    
    float3 _prevPosition;
    float3 _prevRotation;
    
    bool _movedOutsideUpdate;
    bool _rotatedOutsideUpdate;
};
