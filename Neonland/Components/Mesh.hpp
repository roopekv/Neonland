#pragma once

#include "MathUtils.hpp"
#include "ComponentType.hpp"
#include "NeonConstants.h"

class Mesh {
public:
    static constexpr ComponentType componentType = ComponentType::mesh;
    bool operator<(const Mesh& rhs) const {
        return type < rhs.type || (type == rhs.type && texture < rhs.texture);
    }
    
    bool operator>(const Mesh& rhs) const {
        return type > rhs.type || (type == rhs.type && texture > rhs.texture);
    }
    
    bool operator==(const Mesh& rhs) const {
        return type == rhs.type && texture == rhs.texture;
    }
    
    float4x4 modelMatrix;
    MeshType type;
    TextureType texture;
    float4 color;
    
    bool hidden;
    
    float colorMult;
    
    Mesh(MeshType type, TextureType tex = NO_TEX, float4 color = {1, 1, 1, 1}, bool hidden = false, float colorMult = 1);
};
