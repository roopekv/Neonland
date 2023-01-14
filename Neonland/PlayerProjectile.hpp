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
    float lifespan;
    float speed;
    float despawnTime;
    
    PlayerProjectile(int damage = 1, float speed = 100.0f, float lifespan = 2.0f, float despawnTime = 0.0f);
};
