#pragma once

#include <vector>
#include <array>
#include <memory>

#include "IPool.hpp"
#include "ComponentMask.hpp"
#include "Entity.hpp"

class IGroup {
public:
    const ComponentMask requireMask, excludeMask;
    
    IGroup(ComponentMask require, ComponentMask exclude);
    virtual ~IGroup();
    
    auto MatchesGroup(ComponentMask mask) const -> bool {
        return ((mask & requireMask) == requireMask) && ((mask & excludeMask) == 0);
    }
    
    auto GetEntities() const -> const std::vector<Entity>&;
    auto Size() -> size_t;
protected:
    std::vector<Entity> groupEntities;
    auto IsRemoveLocked(const IPool& pool) const -> bool;
    void LockRemove(IPool& pool);
    void UnlockRemove(IPool& pool);
private:
    void AddEntity(Entity entity);
    void AddEntities(const std::vector<Entity>& sortedEntities);
    
    void RemoveEntity(Entity::Id entityId);
    void RemoveEntities(const std::vector<Entity>& sortedEntities);
    
    friend class Scene;
};
