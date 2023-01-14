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
    
    float3 scale;
    
    Transform(float3 pos = {0, 0, 0}, float3 rot = {0, 0, 0}, float3 scale = {1, 1, 1});
    
    void SetPosition(float3 pos);
    void SetRotation(float3 rot);
    
    const float3& GetPosition() const;
    const float3& GetRotation() const;

    friend class NeonScene;
protected:
    float3 _position;
    float3 _rotation;
    
    float3 _prevPosition;
    float3 _prevRotation;
    
    bool _movedOutsideUpdate;
    bool _rotatedOutsideUpdate;
};
