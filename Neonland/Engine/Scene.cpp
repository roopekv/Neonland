#include "Scene.hpp"

Scene::Scene()
: nextEntityId{0}
, releasedEntityCount{0}
, pools(COMPONENT_COUNT, nullptr) {}

void Scene::DestroyEntity(Entity entity) {
    std::scoped_lock lock{destroyMutex};
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

auto Scene::Has(Entity entity, ComponentType type) -> bool {
    auto pool = pools[to_underlying(type)];
    return pool != nullptr && pool->HasComponentFor(entity.id);
}

auto Scene::IsAlive(Entity entity) const -> bool {
    assert(entity.id < sceneEntities.size() && "Entity must be valid");
    return entity == sceneEntities[entity.id];
}

auto Scene::GetMask(const Entity entity) const -> ComponentMask {
    assert(IsAlive(entity) && "Entity must be alive");
    return entityIdToMask[entity.id];
}

auto Scene::ReserveEntity() -> Entity {
    Entity entity;
    if (releasedEntityCount == 0) {
        assert(sceneEntities.size() < Entity::MAX_COUNT && "Number of entities must be less than Entity::MAX_COUNT");
        entity.id = static_cast<Entity::Id>(sceneEntities.size());
        entity.version = 0;
        
        sceneEntities.push_back(entity);
        entityIdToMask.push_back({});
    }
    else {
        entity.id = nextEntityId;
        entity.version = sceneEntities[nextEntityId].version;
        nextEntityId = sceneEntities[nextEntityId].id;
        sceneEntities[entity.id] = entity;
        releasedEntityCount--;
    }
    return entity;
}

void Scene::ReleaseEntity(Entity entity) {
    sceneEntities[entity.id] = {nextEntityId, entity.version + 1};
    nextEntityId = entity.id;
    releasedEntityCount++;
}

auto Scene::GetCount(ComponentType type) const -> size_t {
    auto pool = pools[to_underlying(type)];
    return pool == nullptr ? 0 : pool->size();
}

void Scene::ApplyMaskChanges(Entity entity, ComponentMask changes, bool subtract) {
    auto previousMask = entityIdToMask[entity.id];
    auto newMask = subtract ? previousMask & ~changes : previousMask | changes;
    
    UpdateGroups(entity, newMask);
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
