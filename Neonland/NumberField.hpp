#pragma once

#include <array>
#include <vector>
#include "Entity.hpp"
#include "MathUtils.hpp"
#include "NeonConstants.h"

class NumberField {
public:
    std::array<Entity, 10> valueUIEntities;
    std::array<Entity, 10> maxValueUIEntities;
    std::vector<uint8_t> valueNums;
    std::vector<uint8_t> maxValueNums;
    
    Entity slash;
    
    float2 screenOrigin = {0, 0};
    
    void SetValue(unsigned int value);
    unsigned int GetValue() const;
    
    void SetMaxValue(unsigned int value);
    unsigned int GetMaxValue() const;
private:
    unsigned int value = 0;
    unsigned int maxValue = 0;
};
