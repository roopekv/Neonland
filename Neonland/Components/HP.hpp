#pragma once

#include "ComponentType.hpp"

class HP {
public:
    static constexpr ComponentType componentType = ComponentType::hp;
    bool operator<(const HP& rhs) const {
        return false;
    }
    
    bool operator>(const HP& rhs) const {
        return false;
    }
    
    bool operator==(const HP& rhs) const {
        return true;
    }
    
    int currentHP;
    int maxHP;
    
    HP(int max = 1);
};
