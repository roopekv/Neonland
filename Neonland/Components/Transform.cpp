#include "Transform.hpp"

#include "../NeonConstants.h"

Transform::Transform(float3 pos, float3 rot, float3 scale)
: position{pos}
, rotation{rot}
, scale{scale}
, teleported{false}
, rotationSet{false} {}

void Transform::Teleport(float3 pos) {
    teleported = true;
    position = pos;
}

void Transform::SetRotation(float3 rot) {
    rotationSet = true;
    rotation = rot;
}
