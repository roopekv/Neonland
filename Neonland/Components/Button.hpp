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
    
    bool disabled;
    
    std::function<void()> onClick;
    
    Button(std::function<void()> onClick);
};
