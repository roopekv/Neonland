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

const float3& Transform::GetPosition() const { return _position; };
const float3& Transform::GetRotation() const { return _rotation; };
