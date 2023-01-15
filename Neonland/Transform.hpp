#pragma once
#include "MathUtils.hpp"
#include "ComponentType.hpp"

class Transform {
public:
    static constexpr ComponentType componentType = ComponentType::transform;
    bool operator<(const Transform& rhs) const {
        return false;
    }
    
    bool operator>(const Transform& rhs) const {
        return false;
    }
    
    bool operator==(const Transform& rhs) const {
        return true;
    }
    
    float3 position;
    float3 scale;
    float3 rotation;
    
    bool teleported;
    bool rotationSet;
    
    Transform(float3 pos = {0, 0, 0}, float3 rot = {0, 0, 0}, float3 scale = {1, 1, 1});
    
    void Teleport(float3 pos);
    void SetRotation(float3 rot);
};
