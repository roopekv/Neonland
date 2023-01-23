#pragma once

#include "../Engine/ComponentType.hpp"
#include "../Engine/Entity.hpp"

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
    
    Entity hit = { Entity::NULL_ID, Entity::NULL_VERSION };
    PlayerProjectile(int damage, float lifespan, float speed, bool destructsOnCol);
};
