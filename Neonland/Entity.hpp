#pragma once

#include <simd/simd.h>

#include "MathUtils.h"
#include "ShaderTypes.h"

class Entity {
public:
    vector_float3 vel;
    float angularVel;
    
    int32_t meshIdx;
    
    Entity(vector_float3 pos, float rot, vector_float3 vel, float angularVel, int32_t meshIdx);
    
    void Update(double timestep);
    
    vector_float3& Position();
    float& Rotation();
    
    const vector_float3& Position() const;
    const float& Rotation() const;
    
    matrix_float4x4 Transform(double timeSinceUpdate) const;
    Instance Instance(double timeSinceUpdate) const;
protected:
    vector_float3 _position;
    float _rotation;
    
    bool _movedOutsideUpdate;
    bool _rotatedOutsideUpdate;
};
