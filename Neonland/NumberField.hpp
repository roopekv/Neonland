#pragma once

#include <array>
#include "Entity.hpp"
#include "MathUtils.hpp"

class NumberField {
public:
    std::array<Entity, 10> numberEntities = {};
    std::array<uint8_t, 10> numbers = {};
    
    float2 screenOrigin = {0, 0};
    
    void SetValue(unsigned int value);
    unsigned int GetValue() const;
private:
    unsigned int value;
};
