#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string_view>
#include <string>
#include <algorithm>

#include "ComponentMask.hpp"
#include "Pool.hpp"
#include "Group.hpp"

class Scene {
public:
    Scene();
    
    // Creating entities
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    auto CreateEntity(Args&&... components) -> Entity;
    
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    auto CreateEntity() -> Entity;
    
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    auto CreateEntities(size_t count, Args&&... components) -> std::vector<Entity>;
    
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    auto CreateEntities(size_t count) -> std::vector<Entity>;
    
    // Destroying entities
    void DestroyEntity(Entity entity);
    void DestroyEntities(std::vector<Entity>& entities);
    
    // Entity state
    auto GetMask(Entity entity) const -> ComponentMask;
    
    template<Component T>
    auto HasComponent(Entity entity) const -> bool;
    
    auto IsAlive(Entity entity) const -> bool;
    
    auto IsActive(Entity entity) const -> bool;
    
    void SetActive(Entity entity, bool active);
    void SetActive(const std::vector<Entity>& entities, bool active);
    
    // Adding components
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    void AddComponents(Entity entity, Args&&... components);
    
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    void AddComponents(Entity entity);
    
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    void AddComponents(const std::vector<Entity>& entities, Args&&... components);
    
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    void AddComponents(const std::vector<Entity>& entities);
    
    // Removing components
    template<Component T>
    void RemoveComponent(Entity entity);
    
    template<Component T>
    void RemoveComponent(const std::vector<Entity>& entities);
    
    // Accessing components
    template<Component T>
    auto GetComponent(Entity entity) -> T&;
    
    template<Component T>
    auto GetComponentCount() const -> size_t;
    
    template<Component T>
    auto GetPool() -> std::shared_ptr<Pool<T>>;
    
    // Groups
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    auto CreateGroup(ComponentMask exclude = {}) -> std::shared_ptr<Group<Args...>>;
private:    
    Entity::Id nextEntityId;
    size_t releasedEntityCount;
    
    std::vector<Entity> sceneEntities;
    
    std::vector<ComponentMask> entityIdToMask;
    std::vector<bool> isActive;
    
    std::vector<std::shared_ptr<IPool>> pools;
    std::vector<std::shared_ptr<IGroup>> groups;
    
    auto ReserveEntity() -> Entity;
    auto ReserveEntities(size_t count) -> std::vector<Entity>;
    
    void ReleaseEntity(Entity entity);
    void ReleaseEntities(const std::vector<Entity>& entities);
    
    auto HasComponent(Entity entity, ComponentType type) -> bool;
    
    auto GetComponentCount(ComponentType type) const -> size_t;
    
    void UpdateGroups(Entity entity, ComponentMask newMask);
    void UpdateGroups(const std::vector<Entity>& entitiesWithSameMaskSortedById, ComponentMask newMask);
    
    void ApplyMaskChanges(Entity entity, ComponentMask changes, bool subtract);
    void ApplyMaskChanges(const std::vector<Entity>& entitiesSortedByMask, ComponentMask changes, bool subtract);
    
    template<Component T, Component... Args>
    void AddComponentsToPools(Entity::Id entityId, T&& component, Args&&... components);
    
    template<Component T, Component... Args>
    void AddComponentsToPools(const std::vector<Entity::Id>& sortedEntityIds, T&& component, Args&&... components);
    
    auto GetSortedEntityIds(std::vector<Entity>::const_iterator begin, std::vector<Entity>::const_iterator end) -> std::vector<Entity::Id>;
    
    void SortByMask(std::vector<Entity>& entities);
};

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
auto Scene::CreateEntity(Args&&... components) -> Entity {
    auto entity = ReserveEntity();
    AddComponents(entity, std::forward<Args>(components)...);
    return entity;
}

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
auto Scene::CreateEntity() -> Entity {
    return CreateEntity(Args()...);
}

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
auto Scene::CreateEntities(size_t count, Args&&... components) -> std::vector<Entity> {
    auto entities = ReserveEntities(count);
    AddComponents(entities, std::forward<Args>(components)...);
    return entities;
}

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
auto Scene::CreateEntities(size_t count) -> std::vector<Entity> {
    return CreateEntities(count, Args()...);
}

template<Component T>
auto Scene::HasComponent(Entity entity) const -> bool {
    HasComponent(entity, T::componentType);
}

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
void Scene::AddComponents(Entity entity, Args&&... components) {
    AddComponentsToPools(entity.id, std::forward<Args>(components)...);
    ApplyMaskChanges(entity, GetComponentMask<Args...>(), false);
}

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
void Scene::AddComponents(Entity entity) {
    AddComponents(entity, Args()...);
}

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
void Scene::AddComponents(const std::vector<Entity>& entities, Args&&... components) {
    AddComponentsToPools(GetSortedEntityIds(entities.begin(), entities.end()), std::forward<Args>(components)...);
    ApplyMaskChanges(entities, GetComponentMask<Args...>(), false);
}

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
void Scene::AddComponents(const std::vector<Entity>& entities) {
    AddComponents(entities, Args()...);
}

template<Component T>
void Scene::RemoveComponent(Entity entity) {
    assert(IsAlive(entity) && "Entity must be alive");
    GetPool<T>()->RemoveComponent(entity.id);
    ApplyMaskChanges(entity, GetComponentMask<T>(), true);
}

template<Component T>
void Scene::RemoveComponent(const std::vector<Entity>& entities) {
    GetPool<T>()->RemoveComponents(GetSortedEntityIds(entities.begin(), entities.end()));
    ApplyMaskChanges(entities, GetComponentMask<T>(), true);
}

template<Component T>
auto Scene::GetComponent(Entity entity) -> T& {
    assert(IsAlive(entity) && "Entity must be alive");
    return GetPool<T>()->GetComponent(entity.id);
}

template<Component T>
auto Scene::GetComponentCount() const -> size_t {
    return GetComponentCount(T::componentType);
}

template<Component T>
auto Scene::GetPool() -> std::shared_ptr<Pool<T>> {
    auto type = to_underlying(T::componentType);
    
    if (pools[type] == nullptr) {
        pools[type] = std::make_shared<Pool<T>>();
    }
    
    return std::static_pointer_cast<Pool<T>>(pools[type]);
}

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
auto Scene::CreateGroup(ComponentMask exclude) -> std::shared_ptr<Group<Args...>> {
    auto require = GetComponentMask<Args...>();
    exclude = exclude & ~require;
    
    for (auto group : groups) {
        if (group->requireMask == require && group->excludeMask == exclude) {
            return std::static_pointer_cast<Group<Args...>>(group);
        }
    }
    
    auto group = std::make_shared<Group<Args...>>(exclude, GetPool<Args>()...);
    
    constexpr auto types = std::array<ComponentType, sizeof...(Args)> { Args::componentType... };
    
    auto rarestType = types[0];
    auto minCount = GetComponentCount(types[0]);
    
    for (int i = 1; i < types.size(); i++) {
        auto count = GetComponentCount(types[i]);
        if (count < minCount) {
            minCount = count;
            rarestType = types[i];
        }
    }
    
    if (minCount > 0) {
        auto pool = pools[to_underlying(rarestType)];
        
        for (auto entityId : pool->indexToEntityId) {
            auto mask = entityIdToMask[entityId];
            if (group->MatchesGroup(mask)) {
                group->AddEntity(sceneEntities[entityId]);
            }
        }
    }
    
    groups.push_back(group);
    return group;
}


template<Component T, Component... Args>
void Scene::AddComponentsToPools(Entity::Id entityId, T&& component, Args&&... components) {
    GetPool<T>()->AddComponent(entityId, std::forward<T>(component));
    
    if constexpr (sizeof...(Args) > 0) {
        AddComponentsToPools(entityId, std::forward<Args>(components)...);
    }
}

template<Component T, Component... Args>
void Scene::AddComponentsToPools(const std::vector<Entity::Id>& sortedEntityIds, T&& component, Args&&... components) {
    GetPool<T>()->AddComponents(sortedEntityIds, std::forward<T>(component));
    
    if constexpr (sizeof...(Args) > 0) {
        AddComponentsToPools(sortedEntityIds, std::forward<Args>(components)...);
    }
}
