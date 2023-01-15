#include "Physics.hpp"

Physics::Physics(const Transform& tf, float3 vel, float3 angularVel, float radius)
: velocity{vel}
, angularVelocity{angularVel}
, collisionRadius{radius}
, position{tf.position}
, rotation{tf.rotation}
, prevPosition{tf.position}
, prevRotation{tf.rotation}{}

bool Physics::Overlapping(const Physics& a, const Physics& b,
                        const Transform& tfA, const Transform& tfB) {
    
    float dist = simd_distance(tfA.position, tfB.position);
    float radA = a.collisionRadius * std::min({tfA.scale.x, tfA.scale.y, tfA.scale.z});
    float radB = b.collisionRadius * std::min({tfB.scale.x, tfB.scale.y, tfB.scale.z});
    
    return dist < radA + radB;
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
    
    tf.position = physics.position;
    tf.rotation = physics.rotation;
}

float3 Physics::GetInterpolatedPosition(double interpolation) const {
    return prevPosition + (position - prevPosition) * interpolation;
}

float3 Physics::GetInterpolatedRotation(double interpolation) const {
    return rotation + (rotation - prevRotation) * interpolation;
}
