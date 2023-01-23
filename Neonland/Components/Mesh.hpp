#pragma once

#include "../Engine/MathUtils.hpp"
#include "../Engine/ComponentType.hpp"
#include "../NeonConstants.h"
#include "../Material.hpp"

class Mesh {
public:
    static constexpr ComponentType componentType = ComponentType::mesh;
    bool operator<(const Mesh& rhs) const {
        return type < rhs.type || (type == rhs.type && material < rhs.material);
    }
    
    bool operator>(const Mesh& rhs) const {
        return type > rhs.type || (type == rhs.type && material > rhs.material);
    }
    
    bool operator==(const Mesh& rhs) const {
        return type == rhs.type && material == rhs.material;
    }
    
    float4x4 modelMatrix;
    MeshType type;
    Material material;
    
    float4 tint;
    
    bool hidden;
    
    Mesh(MeshType type, Material material, bool hidden = false, float4 tint = {1, 1, 1, 1});
};
