#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <string_view>
#include <string>
#include <algorithm>
#include <cassert>

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
    
    // Destroying entities
    void DestroyEntity(Entity entity);
    
    // Entity state
    auto GetMask(Entity entity) const -> ComponentMask;
    
    template<Component T>
    auto Has(Entity entity) const -> bool;
    
    auto IsAlive(Entity entity) const -> bool;
    
    // Adding components
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    void Add(Entity entity, Args&&... components);
    
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    void Add(Entity entity);
    
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    void Add(const std::vector<Entity>& entities, Args&&... components);
    
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    void Add(const std::vector<Entity>& entities);
    
    // Removing components
    template<Component T>
    void Remove(Entity entity);
    
    // Accessing components
    template<Component T>
    auto Get(Entity entity) -> T&;
    
    template<Component T>
    auto GetPool() -> std::shared_ptr<Pool<T>>;
    
    // Groups
    template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
    auto GetGroup(ComponentMask exclude = {}) -> std::shared_ptr<Group<Args...>>;
private:    
    Entity::Id nextEntityId;
    size_t releasedEntityCount;
    
    std::mutex destroyMutex;
    
    std::vector<Entity> sceneEntities;
    
    std::vector<ComponentMask> entityIdToMask;
    
    std::vector<std::shared_ptr<IPool>> pools;
    std::vector<std::shared_ptr<IGroup>> groups;
    
    auto ReserveEntity() -> Entity;
    void ReleaseEntity(Entity entity);
    
    auto Has(Entity entity, ComponentType type) -> bool;
    
    auto GetCount(ComponentType type) const -> size_t;
    
    void UpdateGroups(Entity entity, ComponentMask newMask);
    
    void ApplyMaskChanges(Entity entity, ComponentMask changes, bool subtract);
    
    template<Component T, Component... Args>
    void AddComponentsToPools(Entity::Id entityId, T&& component, Args&&... components);
};

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
auto Scene::CreateEntity(Args&&... components) -> Entity {
    auto entity = ReserveEntity();
    Add(entity, std::forward<Args>(components)...);
    return entity;
}

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
auto Scene::CreateEntity() -> Entity {
    return CreateEntity(Args()...);
}

template<Component T>
auto Scene::Has(Entity entity) const -> bool {
    Has(entity, T::componentType);
}

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
void Scene::Add(Entity entity, Args&&... components) {
    AddComponentsToPools(entity.id, std::forward<Args>(components)...);
    ApplyMaskChanges(entity, GetComponentMask<Args...>(), false);
}

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
void Scene::Add(Entity entity) {
    AddComponents(entity, Args()...);
}

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
void Scene::Add(const std::vector<Entity>& entities) {
    AddComponents(entities, Args()...);
}

template<Component T>
void Scene::Remove(Entity entity) {
    assert(IsAlive(entity) && "Entity must be alive");
    GetPool<T>()->RemoveComponent(entity.id);
    ApplyMaskChanges(entity, GetComponentMask<T>(), true);
}

template<Component T>
auto Scene::Get(Entity entity) -> T& {
    assert(IsAlive(entity) && "Entity must be alive");
    return GetPool<T>()->GetComponent(entity.id);
}

template<Component T>
auto Scene::GetPool() -> std::shared_ptr<Pool<T>> {
    constexpr auto type = to_underlying(T::componentType);
    
    if (pools[type] == nullptr) {
        pools[type] = std::make_shared<Pool<T>>();
    }
    
    return std::static_pointer_cast<Pool<T>>(pools[type]);
}

template<Component... Args> requires (sizeof...(Args) > 0 && AllUnique<Args...>)
auto Scene::GetGroup(ComponentMask exclude) -> std::shared_ptr<Group<Args...>> {
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
    auto minCount = GetCount(types[0]);
    
    for (int i = 1; i < types.size(); i++) {
        auto count = GetCount(types[i]);
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
