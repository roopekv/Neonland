#pragma once

#include "ShaderTypes.h"
#include "FrameData.h"

#include "NeonConstants.h"

#ifdef __cplusplus
extern "C" {
#endif

void Neon_Start();
void Neon_UpdateCursorPosition(vector_float2 newPos);
void Neon_UpdateMoveDirection(vector_float2 newDir);

FrameData Neon_Render(float aspectRatio);

#ifdef __cplusplus
}
#endif
