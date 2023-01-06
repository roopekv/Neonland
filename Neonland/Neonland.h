#pragma once

#include <stddef.h>
#include <stdint.h>

#include "ShaderTypes.h"
#include "FrameData.h"
#include "MathUtils.h"

extern const double TIMESTEP;

extern const size_t MAX_ENEMY_COUNT;
extern const size_t MAX_PROJECTILE_COUNT;
extern const size_t MAX_ENTITY_COUNT;

#ifdef __cplusplus
extern "C" {
#endif

void Neonland_Start();
void Neonland_UpdateCursorPosition(vector_float2 newPos);
void Neonland_UpdateMoveDirection(vector_float2 newDir);

FrameData Neonland_Render(float aspectRatio);

#ifdef __cplusplus
}
#endif
