#pragma once

#include "../Engine/MathUtils.hpp"
#include "../Engine/ComponentType.hpp"

class Anchor {
public:
    static constexpr ComponentType componentType = ComponentType::anchor;
    
    bool operator<(const Anchor& rhs) const {
        return false;
    }
    
    bool operator>(const Anchor& rhs) const {
        return false;
    }
    
    bool operator==(const Anchor& rhs) const {
        return true;
    }
    
    float2 screenPosition;
    float3 margin;
    
    Anchor(float2 pos = {0, 0}, float3 margin = {0, 0, 0});
};
