#pragma once

#include <vector>
#include <array>
#include <memory>

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
private:
    void AddEntity(Entity entity);
    void AddEntities(const std::vector<Entity>& sortedEntities);
    
    void RemoveEntity(Entity::Id entityId);
    void RemoveEntities(const std::vector<Entity>& sortedEntities);
    
    friend class Scene;
};
