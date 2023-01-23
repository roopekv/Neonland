#pragma once
#include "../Engine/MathUtils.hpp"
#include "../Engine/ComponentType.hpp"
#include "Transform.hpp"

class Physics {
public:
    static constexpr ComponentType componentType = ComponentType::physics;
    bool operator<(const Physics& rhs) const {
        return false;
    }
    
    bool operator>(const Physics& rhs) const {
        return false;
    }
    
    bool operator==(const Physics& rhs) const {
        return true;
    }
    
    static bool Overlapping(const Physics& a, const Physics& b,
                            const Transform& tfA, const Transform& tfB, float epsilon = 0.0f);
    
    static void Update(Physics& physics, Transform& tf, double timestep);
    
    float3 velocity;
    float3 angularVelocity;
    
    float collisionRadius;
    
    Physics(const Transform& tf, float3 vel = {0, 0, 0}, float3 angularVel = {0, 0, 0}, float radius = 0.5f);
    
    float3 GetInterpolatedPosition(double interpolation) const;
    float3 GetInterpolatedRotation(double interpolation) const;
    
    float GetScaledCollisionRadius(const Transform& tf) const;
    
    float3 position;
    float3 rotation;
    float3 prevPosition;
    float3 prevRotation;
};
