#pragma once

#include "ShaderTypes.h"
#include "FrameData.h"

#include "NeonConstants.h"

enum MeshType {
    PLAYER_MESH = 0,
    ENEMY_MESH = 1
};

#ifdef __cplusplus
extern "C" {
#endif

uint32_t Neon_MeshForGroup(uint32_t groupIdx);

void Neon_Start();
void Neon_UpdateCursorPosition(vector_float2 newPos);
void Neon_UpdateMoveDirection(vector_float2 newDir);

FrameData Neon_Render(float aspectRatio);

#ifdef __cplusplus
}
#endif
