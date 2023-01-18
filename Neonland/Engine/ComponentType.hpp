#pragma once

#include <unordered_map>

template <typename Enum>
constexpr auto to_underlying(Enum e) -> std::underlying_type_t<Enum> {
    return static_cast<std::underlying_type_t<Enum>>(e);
}
enum class ComponentType {
    transform,
    physics,
    camera,
    mesh,
    hp,
    enemy,
    playerProjectile,
    componentTypeCount
};

inline constexpr size_t COMPONENT_COUNT = to_underlying(ComponentType::componentTypeCount);

