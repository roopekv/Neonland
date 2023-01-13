#pragma once

#include <cstdint>
#include <limits>

struct Entity {
    using Id = std::uint32_t;
    using Version = std::uint32_t;
    
    static constexpr auto MAX_COUNT = std::numeric_limits<Id>::max();
    
    static constexpr Id NULL_ID = std::numeric_limits<Id>::max();
    static constexpr Id LAST_VALID_ID = MAX_COUNT - 1;
    
    static constexpr Version NULL_VERSION = std::numeric_limits<Version>::max();
    static constexpr Version LAST_VALID_VERSION = NULL_VERSION - 1;
    
    Id id;
    Version version;
    
    auto operator==(const Entity& rhs) const -> bool = default;
};
