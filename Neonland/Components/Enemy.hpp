#pragma once

#include "ComponentType.hpp"

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
    
    float movementSpeed;
    
    Enemy(int dmg, double cooldown, float moveSpeed);
};
