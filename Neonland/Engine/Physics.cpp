#include "Physics.hpp"

Physics::Physics(float3 vel, float3 angularVel)
: velocity{vel}
, angularVelocity{angularVel} {}

void Physics::Update(Transform& tf, double timestep) {
    tf._prevPosition = tf._position;
    tf._prevRotation = tf._rotation;
    tf._position += velocity * timestep;
    tf._rotation += angularVelocity * timestep;
    tf._movedOutsideUpdate = false;
    tf._rotatedOutsideUpdate = false;
}
