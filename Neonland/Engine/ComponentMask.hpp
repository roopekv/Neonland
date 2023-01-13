#pragma once

#include <bitset>

#include "Component.hpp"

using ComponentMask = std::bitset<COMPONENT_COUNT>;

template<Component T, Component... Args>
inline constexpr auto GetComponentMask(ComponentMask mask = {}) -> ComponentMask {
    mask.set(to_underlying(T::componentType));
    
    if constexpr (sizeof...(Args) > 0) {
        return GetComponentMask<Args...>(mask);
    }
    
    return mask;
}
