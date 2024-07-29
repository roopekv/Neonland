#include "IPool.hpp"

IPool::IPool(ComponentType type)
: componentType{type}
, removeLocked{false} {}

IPool::~IPool() { }

auto IPool::size() const -> size_t {
    return indexToEntityId.size();
}

auto IPool::HasComponentFor(const Entity::Id entityId) const -> bool {
    return entityId < entityIdToIndex.size() && entityIdToIndex[entityId] != DESTROYED;
}

auto IPool::IsRemoveLocked() const -> bool {
    return removeLocked;
}

void IPool::LockRemove() {
    removeLocked = true;
}

void IPool::UnlockRemove() {
    removeLocked = false;
    if (!removeLockedCache.empty()) {
        for (auto entityId : removeLockedCache) {
            RemoveComponent(entityId);
        }
        removeLockedCache.clear();
    }
}
