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
    bool destructsOnCollision;
    
    float despawnTime;
    
    PlayerProjectile(int damage, float lifespan, float speed, bool destructsOnCol);
};
