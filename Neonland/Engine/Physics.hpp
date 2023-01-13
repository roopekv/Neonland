#pragma once
#include "MathUtils.hpp"
#include "ComponentType.hpp"
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
    
    float3 velocity;
    float3 angularVelocity;
    Physics(float3 vel = {0, 0, 0}, float3 angularVel = {0, 0, 0});
    
    void Update(Transform& tf, double timestep);
};
