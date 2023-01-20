#pragma once

#include <functional>
#include "ComponentType.hpp"
#include "MathUtils.hpp"

class Button {
public:
    static constexpr ComponentType componentType = ComponentType::button;
    bool operator<(const Button& rhs) const {
        return false;
    }
    
    bool operator>(const Button& rhs) const {
        return false;
    }
    
    bool operator==(const Button& rhs) const {
        return true;
    }
    
    bool highlighted;
    
    std::function<void()> onClick;
    bool enabled;
    
    Button(std::function<void()> onClick, bool enabled);
};
