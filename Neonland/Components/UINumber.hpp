#pragma once

#include "ComponentType.hpp"
#include "NeonConstants.h"

class UINumber {
public:
    static constexpr ComponentType componentType = ComponentType::uiNumber;
    bool operator<(const UINumber& rhs) const {
        return false;
    }
    
    bool operator>(const UINumber& rhs) const {
        return false;
    }
    
    bool operator==(const UINumber& rhs) const {
        return true;
    }
    
    unsigned int value;
    
    UINumber(unsigned int value);
};
