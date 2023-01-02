#pragma once

#include <simd/simd.h>

#include "MathUtils.h"
#include "ShaderTypes.h"

class Entity {
public:
    vector_float3 vel;
    float angularVel;
    
    uint32_t meshIdx;
    
    Entity(vector_float3 pos, float rot, vector_float3 vel, float angularVel, int32_t meshIdx);
    
    void Update(double timestep);
    
    void SetPosition(vector_float3& pos);
    void SetRotation(float rot);
    
    const vector_float3& GetPosition() const;
    const float& GetRotation() const;
    
    matrix_float4x4 GetTransform(double timeSinceUpdate) const;
    Instance GetInstance(double timeSinceUpdate) const;
protected:
    vector_float3 _position;
    float _rotation;
    
    bool _movedOutsideUpdate;
    bool _rotatedOutsideUpdate;
};
