#pragma once

#include <simd/simd.h>

#include "MathUtils.h"
#include "ShaderTypes.h"

using float3 = simd::float3;

class Entity {
public:
    float3 velocity;
    float3 angularVelocity;
    
    float3 scale;
    
    const uint32_t meshIdx;
    
    Entity(uint32_t meshIdx, float3 pos = {0, 0, 0}, float3 rot = {0, 0, 0}, float3 scale = {1, 1, 1});
    
    void Update(double timestep);
    
    void SetPosition(float3 pos);
    void SetRotation(float3 rot);
    
    const float3& GetPosition() const;
    const float3& GetRotation() const;
    
    simd::float4x4 GetTransform(double timeSinceUpdate) const;
    Instance GetInstance(double timeSinceUpdate) const;
protected:
    float3 _position;
    float3 _rotation;
    
    bool _movedOutsideUpdate;
    bool _rotatedOutsideUpdate;
};
