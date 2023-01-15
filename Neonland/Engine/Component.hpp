#pragma once

#include <concepts>
#include <string>
#include <string_view>
#include <utility>

#include <optional>

#include "ComponentType.hpp"

template<typename T>
concept Component = requires (const T& a, const T& b) {
    { T::componentType } -> std::same_as<const ComponentType&>;
    { a < b } -> std::same_as<bool>;
    { a > b } -> std::same_as<bool>;
    { a == b } -> std::same_as<bool>;
};

template<typename T, typename... Args>
inline constexpr bool AllUnique = sizeof...(Args) == 0 ? true : (!std::is_same_v<T, Args> && ...) && AllUnique<Args...>;

template<typename T>
inline constexpr bool AllUnique<T> = true;
