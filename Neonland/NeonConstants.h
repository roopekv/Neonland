#pragma once

#include <stddef.h>
#include <stdint.h>

extern const double TIMESTEP;

extern const size_t MAX_ENEMY_COUNT;
extern const size_t MAX_PROJECTILE_COUNT;
extern const size_t MAX_INSTANCE_COUNT;

typedef enum MeshType {
    SPHERE_MESH,
    CUBE_MESH,
    PLANE_MESH,
    MeshTypeCount
} MeshType;
