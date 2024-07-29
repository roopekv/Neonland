#include "IGroup.hpp"

#include <numeric>

IGroup::IGroup(ComponentMask require, ComponentMask exclude)
: requireMask{require}
, excludeMask{exclude & ~requireMask} { }

IGroup::~IGroup() { }

auto IGroup::Size() -> size_t {
    return groupEntities.size();
}

void IGroup::AddEntity(Entity entity) {
    auto upperBound = std::upper_bound(groupEntities.begin(), groupEntities.end(), entity, [](auto a, auto b) {
        return a.id < b.id;
    });
    groupEntities.insert(upperBound, entity);
}

void IGroup::AddEntities(const std::vector<Entity>& sortedEntities) {
    size_t beginIndex = 0;
    for (size_t i = 1; i < sortedEntities.size(); i++) {
        if (sortedEntities[i - 1].id != sortedEntities[i].id - 1) {
            auto upperBound = std::upper_bound(groupEntities.begin(), groupEntities.end(), sortedEntities[beginIndex], [](auto a, auto b) {
                return a.id < b.id;
            });
            
            groupEntities.insert(upperBound, sortedEntities.begin() + beginIndex, sortedEntities.begin() + i);
            beginIndex = i;
        }
    }
    
    auto upperBound = std::upper_bound(groupEntities.begin(), groupEntities.end(), sortedEntities[beginIndex], [](auto a, auto b) {
        return a.id < b.id;
    });
    
    groupEntities.insert(upperBound, sortedEntities.begin() + beginIndex, sortedEntities.end());
}

void IGroup::RemoveEntity(Entity::Id entityId) {
    auto lowerBound = std::lower_bound(groupEntities.begin(), groupEntities.end(), entityId, [](auto entity, auto id) {
        return entity.id < id;
    });
    groupEntities.erase(lowerBound);
}

void IGroup::RemoveEntities(const std::vector<Entity>& sortedEntities) {
    size_t beginIndex = 0;
    for (size_t i = 1; i < sortedEntities.size(); i++) {
        if (sortedEntities[i - 1].id != sortedEntities[i].id - 1) {
            auto lowerBound = std::lower_bound(groupEntities.begin(), groupEntities.end(), sortedEntities[beginIndex], [](auto a, auto b) {
                return a.id < b.id;
            });
            
            groupEntities.erase(lowerBound, lowerBound + (i - beginIndex));
            beginIndex = i;
        }
    }
    
    auto lowerBound = std::lower_bound(groupEntities.begin(), groupEntities.end(), sortedEntities[beginIndex], [](auto a, auto b) {
        return a.id < b.id;
    });
    
    groupEntities.erase(lowerBound, lowerBound + (sortedEntities.size() - beginIndex));
}

auto IGroup::GetEntities() const -> const std::vector<Entity>& {
    return groupEntities;
}

auto IGroup::IsRemoveLocked(const IPool& pool) const -> bool {
    return pool.IsRemoveLocked();
}

void IGroup::LockRemove(IPool& pool) {
    pool.LockRemove();
}

void IGroup::UnlockRemove(IPool& pool) {
    pool.UnlockRemove();
}
