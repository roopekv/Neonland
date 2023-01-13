#include "Scene.hpp"

Scene::Scene()
: nextEntityId{0}
, releasedEntityCount{0}
, pools(COMPONENT_COUNT, nullptr) {}

void Scene::DestroyEntity(Entity entity) {
    auto mask = entityIdToMask[entity.id];
    
    size_t n = 0;
    for (size_t i = 0; n < mask.count(); i++) {
        if (mask[i]) {
            pools[i]->RemoveComponent(entity.id);
            n++;
        }
    }
    
    UpdateGroups(entity, {});
    ReleaseEntity(entity);
}

void Scene::DestroyEntities(std::vector<Entity>& entities) {
    SortByMask(entities);
    
    auto prevMask = entityIdToMask[entities.front().id];
    size_t startIndex = 0;
    
    for (size_t i = 1; i < entities.size(); i++) {
        auto mask = entityIdToMask[entities[i].id];
        if (mask != prevMask) {
            auto sortedEntityIds= GetSortedEntityIds(entities.begin() + startIndex, entities.begin() + i);
            size_t removedCount = 0;
            for (size_t componentIndex = 0; removedCount < prevMask.count(); componentIndex++) {
                if (prevMask[componentIndex]) {
                    pools[componentIndex]->RemoveComponents(sortedEntityIds);
                    removedCount++;
                }
            }
            
            prevMask = mask;
        }
    }
    
    auto sortedEntityIds= GetSortedEntityIds(entities.begin() + startIndex, entities.end());
    size_t removedCount = 0;
    for (size_t componentIndex = 0; removedCount < prevMask.count(); componentIndex++) {
        if (prevMask[componentIndex]) {
            pools[componentIndex]->RemoveComponents(sortedEntityIds);
            removedCount++;
        }
    }
    
    UpdateGroups(entities, {});
    ReleaseEntities(entities);
    entities.clear();
}

auto Scene::HasComponent(Entity entity, ComponentType type) -> bool {
    auto pool = pools[to_underlying(type)];
    return pool != nullptr && pool->HasComponentFor(entity.id);
}

auto Scene::IsAlive(Entity entity) const -> bool {
    assert(entity.id < sceneEntities.size() && "Entity must be valid");
    return entity == sceneEntities[entity.id];
}

auto Scene::IsActive(Entity entity) const -> bool {
    assert(IsAlive(entity) && "Entity must be alive");
    return isActive[entity.id];
}

void Scene::SetActive(Entity entity, bool active) {
    assert(IsAlive(entity) && "Entity must be alive");
    isActive[entity.id] = active;
}

void Scene::SetActive(const std::vector<Entity>& entities, bool active) {
    for (auto entity : entities) {
        SetActive(entity, active);
    }
}

auto Scene::GetMask(const Entity entity) const -> ComponentMask {
    assert(IsAlive(entity) && "Entity must be alive");
    return entityIdToMask[entity.id];
}

auto Scene::ReserveEntity() -> Entity {
    if (releasedEntityCount == 0) {
        assert(sceneEntities.size() < Entity::MAX_COUNT && "Number of entities must be less than Entity::MAX_COUNT");
        auto entity = Entity{static_cast<Entity::Id>(sceneEntities.size()), 0};
        sceneEntities.push_back(entity);
        
        entityIdToMask.push_back({});
        isActive.push_back(true);
        
        return entity;
    }
    
    Entity entity{nextEntityId, sceneEntities[nextEntityId].version};
    nextEntityId = sceneEntities[nextEntityId].id;
    releasedEntityCount--;
    
    isActive[entity.id] = true;
    
    return entity;
}

auto Scene::ReserveEntities(size_t count) -> std::vector<Entity> {
    std::vector<Entity> entities;
    entities.reserve(count);
    
    auto recycleCount = releasedEntityCount > count ? count : releasedEntityCount;
    count -= recycleCount;
    releasedEntityCount -= recycleCount;
    
    while (entities.size() < recycleCount) {
        Entity entity{nextEntityId, sceneEntities[nextEntityId].version};
        nextEntityId = sceneEntities[nextEntityId].id;
        isActive[entity.id] = true;
        
        entities.push_back(entity);
    }
    
    auto newEntityCount = sceneEntities.size() + count;
    assert(newEntityCount < Entity::MAX_COUNT && "Number of entities must be less than Entity::MAX_COUNT");
    
    sceneEntities.reserve(newEntityCount);
    isActive.resize(newEntityCount, true);
    entityIdToMask.resize(newEntityCount, {});
    
    for (size_t i = sceneEntities.size(); i < newEntityCount; i++) {
        Entity newEntity{static_cast<Entity::Id>(i), 0};
        sceneEntities.push_back(newEntity);
        entities.push_back(newEntity);
    }
    
    return entities;
}

void Scene::ReleaseEntity(Entity entity) {
    sceneEntities[entity.id] = {nextEntityId, entity.version + 1};
    nextEntityId = entity.id;
    releasedEntityCount++;
}

void Scene::ReleaseEntities(const std::vector<Entity>& entities) {
    for (auto entity : entities) {
        sceneEntities[entity.id] = {nextEntityId, entity.version + 1};
        nextEntityId = entity.id;
    }
    
    releasedEntityCount += entities.size();
}

auto Scene::GetComponentCount(ComponentType type) const -> size_t {
    auto pool = pools[to_underlying(type)];
    return pool == nullptr ? 0 : pool->size();
}

void Scene::ApplyMaskChanges(Entity entity, ComponentMask changes, bool subtract) {
    auto previousMask = entityIdToMask[entity.id];
    auto newMask = subtract ? previousMask & ~changes : previousMask | changes;
    
    UpdateGroups(entity, newMask);
}

void Scene::ApplyMaskChanges(const std::vector<Entity>& entitiesSortedByMask, ComponentMask changes, bool subtract) {
    auto prevMask = entityIdToMask[entitiesSortedByMask.front().id];
    size_t startIndex = 0;
    
    for (size_t i = 1; i < entitiesSortedByMask.size(); i++) {
        auto entityMask = entityIdToMask[entitiesSortedByMask[i].id];
        
        if (entityMask != prevMask) {
            std::vector<Entity> entitiesWithSameMaskSortedById(entitiesSortedByMask.begin() + startIndex, entitiesSortedByMask.begin() + i);
            std::sort(entitiesWithSameMaskSortedById.begin(), entitiesWithSameMaskSortedById.end(), [](auto a, auto b) {
                return a.id < b.id;
            });
            
            auto newMask = subtract ? prevMask & ~changes : prevMask | changes;
            UpdateGroups(entitiesWithSameMaskSortedById, newMask);
            
            for (auto entity : entitiesWithSameMaskSortedById) {
                entityIdToMask[entity.id] = newMask;
            }
            
            prevMask = entityMask;
            startIndex = i;
        }
    }
    
    std::vector<Entity> entitiesWithSameMask(entitiesSortedByMask.begin() + startIndex, entitiesSortedByMask.end());
    std::sort(entitiesWithSameMask.begin(), entitiesWithSameMask.end(), [](auto a, auto b) {
        return a.id < b.id;
    });
    
    auto newMask = subtract ? prevMask & ~changes : prevMask | changes;
    UpdateGroups(entitiesWithSameMask, newMask);
    
    for (auto entity : entitiesWithSameMask) {
        entityIdToMask[entity.id] = newMask;
    }
}

void Scene::UpdateGroups(Entity entity, ComponentMask newMask) {
    for (auto group : groups) {
        bool matchesPrev = group->MatchesGroup(entityIdToMask[entity.id]);
        bool matchesNew = group->MatchesGroup(newMask);
        
        if (matchesNew && !matchesPrev) {
            group->AddEntity(entity);
        }
        else if (matchesPrev && !matchesNew) {
            group->RemoveEntity(entity.id);
        }
    }
    
    entityIdToMask[entity.id] = newMask;
}

void Scene::UpdateGroups(const std::vector<Entity>& entitiesWithSameMaskSortedById, const ComponentMask newMask) {
    const auto prevMask = entityIdToMask[entitiesWithSameMaskSortedById.front().id];
    
    for (auto group : groups) {
        bool matchesPrev = group->MatchesGroup(prevMask);
        bool matchesNew = group->MatchesGroup(newMask);
        
        if (matchesNew && !matchesPrev) {
            group->AddEntities(entitiesWithSameMaskSortedById);
        }
        else if (matchesPrev && !matchesNew) {
            group->RemoveEntities(entitiesWithSameMaskSortedById);
        }
    }
}

auto Scene::GetSortedEntityIds(std::vector<Entity>::const_iterator begin, std::vector<Entity>::const_iterator end) -> std::vector<Entity::Id> {
    std::vector<Entity::Id> sortedEntityIds;
    sortedEntityIds.reserve(end - begin);
    
    std::transform(begin, end, std::back_inserter(sortedEntityIds), [](auto entity) {
        return entity.id;
    });
    
    std::sort(sortedEntityIds.begin(), sortedEntityIds.end());
    
    return sortedEntityIds;
}

void Scene::SortByMask(std::vector<Entity>& entities) {
    std::sort(entities.begin(), entities.end(), [this](auto a, auto b) {
        auto maskValueA = entityIdToMask[a.id].to_ullong();
        auto maskValueB = entityIdToMask[b.id].to_ullong();
        
        return maskValueA < maskValueB;
    });
}
