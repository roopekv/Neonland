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
    
    Enemy(int dmg = 1, double cooldown = 0.5f);
};
