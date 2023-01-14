#pragma once

#include <vector>
#include <cstdint>
#include <limits>
#include <string_view>
#include <string>

#include "Entity.hpp"
#include "ComponentType.hpp"

class IPool {
public:
    virtual ~IPool();
    auto size() const -> size_t;
protected:
    using Index = Entity::Id;
    static constexpr Index DESTROYED = Entity::NULL_ID;
    
    std::vector<Index> entityIdToIndex;
    std::vector<Entity::Id> indexToEntityId;
    
    const ComponentType componentType;
    
    IPool(ComponentType type);
    
    auto HasComponentFor(Entity::Id entityId) const -> bool;
    
    virtual void RemoveComponent(Entity::Id entityId) = 0;
    
    friend class Scene;
};
