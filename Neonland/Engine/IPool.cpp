#include "IPool.hpp"

IPool::IPool(ComponentType type)
: componentType{type} {}

IPool::~IPool() { }

auto IPool::size() const -> size_t {
    return indexToEntityId.size();
}

auto IPool::HasComponentFor(const Entity::Id entityId) const -> bool {
    return entityId < entityIdToIndex.size() && entityIdToIndex[entityId] != DESTROYED;
}
