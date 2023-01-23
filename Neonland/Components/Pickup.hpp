#pragma once

#include "../Engine/ComponentType.hpp"

class Pickup {
public:
    enum Type {
        Health,
        ThreeSixtyShots
    };
    
    static constexpr ComponentType componentType = ComponentType::pickup;
    
    bool operator<(const Pickup& rhs) const {
        return false;
    }
    
    bool operator>(const Pickup& rhs) const {
        return false;
    }
    
    bool operator==(const Pickup& rhs) const {
        return true;
    }
    
    Type type;
    
    Pickup(Type type);
};
