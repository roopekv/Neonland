#pragma once

#include <array>
#include <vector>
#include "Entity.hpp"
#include "MathUtils.hpp"
#include "NeonConstants.h"

class NumberField {
public:
    std::array<Entity, 10> valueUIEntities;
    std::vector<int32_t> valueNums;
    
    bool textFirst = false;
    Entity text;
    
    void SetValue(int32_t value);
    int32_t GetValue() const;
private:
    int32_t value = 0;
};
