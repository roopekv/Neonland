#include "Entity.hpp"

#include "NeonConstants.h"

Entity::Entity(uint32_t type, float3 pos, float3 rot, float3 scale)
: type(type)
, _position{pos}
, _prevRotation(pos)
, _rotation{rot}
, _prevPosition(pos)
, scale{scale}
, velocity{0, 0}
, angularVelocity{0}
, _movedOutsideUpdate{false}
, _rotatedOutsideUpdate{false} {}

void Entity::Update(double timestep) {
    _prevPosition = _position;
    _position += velocity * timestep;
    _rotation += angularVelocity * timestep;
    _movedOutsideUpdate = false;
    _rotatedOutsideUpdate = false;
}

void Entity::SetPosition(float3 pos) {
    _movedOutsideUpdate = true;
    _position = pos;
}

void Entity::SetRotation(float3 rot) {
    _rotatedOutsideUpdate = true;
    _rotation = rot;
}

const float3& Entity::GetPosition() const { return _position; };
const float3& Entity::GetRotation() const { return _rotation; };

float4x4 Entity::GetTransform(double timeSinceUpdate) const {
    constexpr static auto xAxis = float3{1, 0, 0};
    constexpr static auto yAxis = float3{0, 1, 0};
    constexpr static auto zAxis = float3{0, 0, 1};
    
    float4x4 T, R, S;

    S = ScaleMatrix(scale);
    
    float4x4 rX;
    float4x4 rY;
    float4x4 rZ;
    
    if (_movedOutsideUpdate) {
        T = TranslationMatrix(_position);
        
        rX = RotationMatrix(xAxis, _rotation.x);
        rY = RotationMatrix(yAxis, _rotation.y);
        rZ = RotationMatrix(zAxis, _rotation.z);
    }
    else {
        
        T = TranslationMatrix(_prevPosition + (_position - _prevPosition) * (timeSinceUpdate / TIMESTEP));
        
        float3 r = _rotation + angularVelocity * timeSinceUpdate;
        rX = RotationMatrix(xAxis, r.x);
        rY = RotationMatrix(yAxis, r.y);
        rZ = RotationMatrix(zAxis, r.z);
    }
    
    R = rZ * rY * rX;
    
    return T * R * S;
}

Instance Entity::GetInstance(double timeSinceUpdate) const {
    Instance instance;
    instance.transform = GetTransform(timeSinceUpdate);
    return instance;
}
