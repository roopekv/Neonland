#pragma once

#include <memory>

#include "IPool.hpp"
#include "Component.hpp"

template<Component T>
class Pool : public IPool {
public:
    using iterator = typename std::vector<T>::iterator;
    
    Pool<T>();
    
    auto operator[](size_t index) -> T&;
    auto operator[](size_t index) const -> const T&;
    
    auto begin() -> iterator;
    auto begin() const -> const iterator;
    auto cbegin() const -> const iterator;
    
    auto end() -> iterator;
    auto end() const -> const iterator;
    auto cend() const -> const iterator;
    
    auto back() -> T&;
    auto back() const -> const T&;
    
    auto GetComponent(Entity::Id entityId) -> T&;
private:
    std::vector<T> components;
    
    bool removeLocked;
    std::vector<Index> removeLockedCache;
    
    void RemoveComponent(Entity::Id entityId) override final;
    void RemoveComponents(const std::vector<Entity::Id>& sortedEntityIds) override final;
    
    void AddComponent(Entity::Id entityId, T&& component);
    void AddComponents(const std::vector<Entity::Id>& sortedEntityIds, T&& component);
    
    auto IsRemoveLocked() -> bool;
    
    void LockRemove();
    void UnlockRemove();
    
    friend class Scene;
    
    template<Component... Args> requires (sizeof...(Args) > 0) && AllUnique<Args...>
    friend class Group;
};

template<Component T>
Pool<T>::Pool()
: removeLocked{false}
, IPool(T::componentType) {}

template<Component T>
auto Pool<T>::operator[](size_t index) -> T& {
    return components[index];
}

template<Component T>
auto Pool<T>::operator[](size_t index) const -> const T& {
    return components[index];
}

template<Component T>
auto Pool<T>::begin() -> iterator {
    return components.begin();
}

template<Component T>
auto Pool<T>::begin() const -> const iterator {
    return components.begin();
}

template<Component T>
auto Pool<T>::cbegin() const -> const iterator {
    return components.cbegin();
}

template<Component T>
auto Pool<T>::end() -> iterator {
    return components.end();
}

template<Component T>
auto Pool<T>::end() const -> const iterator {
    return components.end();
}

template<Component T>
auto Pool<T>::cend() const -> const iterator {
    return components.cend();
}

template<Component T>
auto Pool<T>::back() -> T& {
    return components.back();
}

template<Component T>
auto Pool<T>::back() const -> const T& {
    return components.back();
}

template<Component T>
void Pool<T>::AddComponent(const Entity::Id entityId, T&& component) {
    if (entityId < entityIdToIndex.size()) {
        if (entityIdToIndex[entityId] != DESTROYED) {
            return;
        }
    }
    else {
        entityIdToIndex.resize(entityId + 1, DESTROYED);
    }
    
    Index index = static_cast<Index>(std::upper_bound(components.begin(), components.end(), component) - components.begin());
    entityIdToIndex[entityId] = index;
    for (Index i = index; i < indexToEntityId.size(); i++) {
        entityIdToIndex[i]++;
    }
    indexToEntityId.insert(indexToEntityId.begin() + index, entityId);
    components.insert(components.begin() + index, component);
}

template<Component T>
void Pool<T>::AddComponents(const std::vector<Entity::Id>& sortedEntityIds, T&& component) {
    if (sortedEntityIds.empty()) {
        return;
    }
    
    assert(!sortedEntityIds.empty() && "Entities must not be empty");
    
    auto requiredSize = sortedEntityIds.back() + 1;
    if (entityIdToIndex.size() < requiredSize) {
        entityIdToIndex.resize(requiredSize, DESTROYED);
    }
    
    Index index = static_cast<Index>(std::upper_bound(components.begin(), components.end(), component) - components.begin());
    Index nextIndex = index;
    for (auto entityId : sortedEntityIds) {
        if (entityIdToIndex[entityId] == DESTROYED) {
            entityIdToIndex[entityId] = nextIndex++;
            indexToEntityId.push_back(entityId);
        }
    }
    
    Index newCount = nextIndex - index;
    for (Index i = nextIndex; i < indexToEntityId.size(); i++) {
        entityIdToIndex[i] += newCount;
    }
    
    std::vector<T> tmp(newCount, component);
    components.insert(components.begin() + index,
                      std::make_move_iterator(tmp.begin()),
                      std::make_move_iterator(tmp.end()));
}

template<Component T>
void Pool<T>::RemoveComponent(Entity::Id entityId) {
    assert(HasComponentFor(entityId) && "Entity must have the specified component");
    
    if (removeLocked) {
        removeLockedCache.push_back(entityIdToIndex[entityId]);
        entityIdToIndex[entityId] = DESTROYED;
    }
    else if (components.back() == components[entityIdToIndex[entityId]]){
        auto backEntityId = indexToEntityId.back();
        auto index = entityIdToIndex[entityId];
        
        indexToEntityId[index] = backEntityId;
        entityIdToIndex[backEntityId] = index;
        
        std::swap(components[index], components.back());
        
        indexToEntityId.pop_back();
        components.pop_back();
        entityIdToIndex[entityId] = DESTROYED;
    }
    else {
        Index index = entityIdToIndex[entityId];
        components.erase(components.begin() + index);
        indexToEntityId.erase(indexToEntityId.begin() + index);
        for (Index i = index; i < indexToEntityId.size(); i++) {
            entityIdToIndex[i] -= 1;
        }
        
        entityIdToIndex[entityId] = DESTROYED;
    }
}

template<Component T>
void Pool<T>::RemoveComponents(const std::vector<Entity::Id>& sortedEntityIds) {
    // Can be optimized
    for (auto entityId : sortedEntityIds) {
        RemoveComponent(entityId);
    }
}

template<Component T>
auto Pool<T>::GetComponent(Entity::Id entityId) -> T& {
    return components[entityIdToIndex[entityId]];
}

template<Component T>
auto Pool<T>::IsRemoveLocked() -> bool {
    return removeLocked;
}

template<Component T>
void Pool<T>::LockRemove() {
    removeLocked = true;
}

template<Component T>
void Pool<T>::UnlockRemove() {
    removeLocked = false;
    for (auto index : removeLockedCache) {
        auto backEntityId = indexToEntityId.back();
        
        indexToEntityId[index] = backEntityId;
        entityIdToIndex[backEntityId] = index;
        
        std::swap(components[index], components.back());
        
        indexToEntityId.pop_back();
        components.pop_back();
    }
}
