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
    CROSSHAIR_MESH,
    PLANE_MESH,
    SPREAD_MESH,
    SHARD_MESH,
    MeshTypeCount
} MeshType;

typedef enum ShaderType {
    LIT_SHADER,
    UI_SHADER,
    ShaderTypeCount
} ShaderType;

typedef enum TextureType {
    NO_TEX,
    GROUND0_TEX,
    GROUND1_TEX,
    GROUND2_TEX,
    NEONLAND_TEX,
    WAVE_TEX,
    HP_TEX,
    ENEMIES_REMAINING_TEX,
    LEVEL_TEX,
    NUM_KEYS_TEX,
    ZERO_TEX,
    ONE_TEX,
    TWO_TEX,
    THREE_TEX,
    FOUR_TEX,
    FIVE_TEX,
    SIX_TEX,
    SEVEN_TEX,
    EIGHT_TEX,
    NINE_TEX,
    TextureTypeCount,
} TextureType;
