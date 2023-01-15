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

typedef enum TextureType {
    ZERO_TEX =  0,
    ONE_TEX =   1,
    TWO_TEX =   2,
    THREE_TEX = 3,
    FOUR_TEX =  4,
    FIVE_TEX =  5,
    SIX_TEX =   6,
    SEVEN_TEX = 7,
    EIGHT_TEX = 8,
    NINE_TEX =  9,
    GROUND_TEX,
    TextureTypeCount,
    NO_TEX
} TextureType;
