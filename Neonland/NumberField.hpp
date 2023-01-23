#pragma once

#include <array>
#include <vector>
#include "./Engine/Entity.hpp"
#include "./Engine/MathUtils.hpp"
#include "NeonConstants.h"

class NumberField {
public:
    std::array<Entity, 10> digitEntities;
    Entity tex;
    
    std::vector<int32_t> digits;
    
    bool hidden = false;
    bool texFirst = false;
    
    float2 screenPos = {0, 0};
    
    void SetValue(int32_t value);
    int32_t GetValue() const;
private:
    int32_t value = 0;
};
