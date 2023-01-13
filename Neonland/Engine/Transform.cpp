#include "Transform.hpp"

#include "NeonConstants.h"

Transform::Transform(float3 pos, float3 rot, float3 scale)
: _position{pos}
, _prevRotation(pos)
, _rotation{rot}
, _prevPosition(pos)
, scale{scale}
, _movedOutsideUpdate{false}
, _rotatedOutsideUpdate{false} {}

void Transform::SetPosition(float3 pos) {
    _movedOutsideUpdate = true;
    _position = pos;
}

void Transform::SetRotation(float3 rot) {
    _rotatedOutsideUpdate = true;
    _rotation = rot;
}

float4x4 Transform::GetMatrix(double interpolation) const {
    constexpr static auto xAxis = float3{1, 0, 0};
    constexpr static auto yAxis = float3{0, 1, 0};
    constexpr static auto zAxis = float3{0, 0, 1};
    
    float4x4 T, R, S;
    
    S = ScaleMatrix(scale);
    
    T = TranslationMatrix(_position);
    
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
        T = TranslationMatrix(_prevPosition + (_position - _prevPosition) * interpolation);
        
        float3 r = _rotation + (_rotation - _prevRotation) * interpolation;
        rX = RotationMatrix(xAxis, r.x);
        rY = RotationMatrix(yAxis, r.y);
        rZ = RotationMatrix(zAxis, r.z);
    }
    
    R = rZ * rY * rX;
    
    return T * R * S;
}

const float3& Transform::GetPosition() const { return _position; };
const float3& Transform::GetRotation() const { return _rotation; };
