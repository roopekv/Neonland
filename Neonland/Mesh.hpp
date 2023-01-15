#pragma once

#include "MathUtils.hpp"
#include "ComponentType.hpp"
#include "NeonConstants.h"

class Mesh {
public:
    static constexpr ComponentType componentType = ComponentType::mesh;
    bool operator<(const Mesh& rhs) const {
        return type < rhs.type;
    }
    
    bool operator>(const Mesh& rhs) const {
        return type > rhs.type;
    }
    
    bool operator==(const Mesh& rhs) const {
        return type == rhs.type;
    }
    
    enum Type : uint32_t {
        PLAYER = SPHERE_MESH,
        PROJECTILE = SPHERE_MESH,
        CROSSHAIR = SPHERE_MESH,
        ENEMY = CUBE_MESH,
        PLANE = PLANE_MESH
    };
    
    float4x4 modelMatrix;
    Type type;
    
    Mesh(Type type);
};
