#pragma once

#include "../Engine/ComponentType.hpp"

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
    
    
    HP(int max = 1);
    
    void Set(int hp);
    
    int Get() const;
    
    int Max() const;
    
    void Decrease(int dmg);
    void Increase(int heal);
    
private:
    int hp;
    int maxHP;
};
