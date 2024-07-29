#pragma once

#include <memory>

#include "IPool.hpp"
#include "Component.hpp"
#include <mutex>

#include <iostream>

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
    
    void Sort();
private:
    std::vector<T> components;
    
    void RemoveComponent(Entity::Id entityId) override final;
    void AddComponent(Entity::Id entityId, T&& component);
    
    friend class Scene;
};

template<Component T>
Pool<T>::Pool()
: IPool(T::componentType) {}

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
    assert(!(entityId < entityIdToIndex.size() && entityIdToIndex[entityId] != DESTROYED) && "Component cannot be added twice");
    
    if (entityId + 1 > entityIdToIndex.size()) {
        entityIdToIndex.resize(entityId + 1, DESTROYED);
    }
    
    Index index = static_cast<Index>(std::upper_bound(components.begin(), components.end(), component) - components.begin());
    entityIdToIndex[entityId] = index;
    for (Index i = index; i < indexToEntityId.size(); i++) {
        entityIdToIndex[indexToEntityId[i]]++;
    }
    indexToEntityId.insert(indexToEntityId.begin() + index, entityId);
    components.insert(components.begin() + index, component);
}

template<Component T>
void Pool<T>::RemoveComponent(Entity::Id entityId) {
    assert(HasComponentFor(entityId) && "Entity must have the specified component");
    
    if (removeLocked) {
        removeLockedCache.push_back(entityId);
    }
    else {
        Index index = entityIdToIndex[entityId];
        components.erase(components.begin() + index);
        indexToEntityId.erase(indexToEntityId.begin() + index);
        for (Index i = index; i < indexToEntityId.size(); i++) {
            entityIdToIndex[indexToEntityId[i]]--;
        }
        entityIdToIndex[entityId] = DESTROYED;
    }
}

template<Component T>
auto Pool<T>::GetComponent(Entity::Id entityId) -> T& {
    return components[entityIdToIndex[entityId]];
}

template<Component T>
void Pool<T>::Sort() {
    std::sort(indexToEntityId.begin(), indexToEntityId.end(), [this](auto a, auto b) {
        return components[entityIdToIndex[a]] < components[entityIdToIndex[b]];
    });
    
    std::sort(components.begin(), components.end());
    
    for (Index idx = 0; idx < indexToEntityId.size(); idx++) {
        entityIdToIndex[indexToEntityId[idx]] = idx;
    }
}
