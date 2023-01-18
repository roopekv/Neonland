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
    std::vector<int32_t> valueNums;
    std::vector<int32_t> maxValueNums;
    
    Entity slash;
    
    float2 origin = {0, 0};
    
    void SetValue(int32_t value);
    int32_t GetValue() const;
    
    void SetMaxValue(int32_t value);
    int32_t GetMaxValue() const;
private:
    int32_t value = 0;
    int32_t maxValue = 0;
};
