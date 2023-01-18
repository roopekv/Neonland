#pragma once

#include "ComponentType.hpp"
#include "MathUtils.hpp"

class Enemy {
public:
    static constexpr ComponentType componentType = ComponentType::enemy;
    bool operator<(const Enemy& rhs) const {
        return false;
    }
    
    bool operator>(const Enemy& rhs) const {
        return false;
    }
    
    bool operator==(const Enemy& rhs) const {
        return true;
    }
    
    int attackDamage;
    double attackCooldown;
    double cooldownEndTime;
    
    float maxMovementSpeed;
    float acceleration;
    
    Enemy(int dmg, double cooldown, float maxSpeed, float acceleration = 1.0f);
};
