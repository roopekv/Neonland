#include "Physics.hpp"
#include <iostream>
#include <algorithm>

Physics::Physics(const Transform& tf, float3 vel, float3 angularVel, float radius)
: velocity{vel}
, angularVelocity{angularVel}
, collisionRadius{radius}
, position{tf.position}
, rotation{tf.rotation}
, prevPosition{tf.position}
, prevRotation{tf.rotation} {}

bool Physics::Overlapping(const Physics& a, const Physics& b,
                        const Transform& tfA, const Transform& tfB, float epsilon) {
    float3 aToB = a.position - b.position;
    aToB.z = 0;
    
    float dist = VecLength(aToB);
    float radA = a.GetScaledCollisionRadius(tfA);
    float radB = b.GetScaledCollisionRadius(tfB);
    
    return dist - epsilon < radA + radB;
}

void Physics::Update(Physics& physics, Transform& tf, double timestep) {
    physics.prevPosition = physics.position;
    physics.prevRotation = physics.rotation;
    
    if (tf.teleported) {
        physics.prevPosition = tf.position;
        physics.position = tf.position;
        tf.teleported = false;
    }
    
    if (tf.rotationSet) {
        physics.prevRotation = tf.rotation;
        physics.rotation = tf.rotation;
        tf.rotationSet = false;
    }
    
    physics.position += physics.velocity * timestep;
    physics.rotation += physics.angularVelocity * timestep;
}

float Physics::GetScaledCollisionRadius(const Transform& tf) const {
    return collisionRadius * std::min({tf.scale.x, tf.scale.y, tf.scale.z});
}

float3 Physics::GetInterpolatedPosition(double interpolation) const {
    return prevPosition + (position - prevPosition) * interpolation;
}

float3 Physics::GetInterpolatedRotation(double interpolation) const {
    return prevRotation + (rotation - prevRotation) * interpolation;
}
