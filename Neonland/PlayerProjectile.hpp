#pragma once

#include"ComponentType.hpp"

class PlayerProjectile {
public:
    static constexpr ComponentType componentType = ComponentType::playerProjectile;
    bool operator<(const PlayerProjectile& rhs) const {
        return false;
    }
    
    bool operator>(const PlayerProjectile& rhs) const {
        return false;
    }
    
    bool operator==(const PlayerProjectile& rhs) const {
        return true;
    }
    
    int damage;
    float despawnTime;
    
    PlayerProjectile(int damage, float despawnTime);
};
