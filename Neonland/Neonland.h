#pragma once

#include "ShaderTypes.h"
#include "FrameData.h"

#include "NeonConstants.h"

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

void Neon_UpdateCursorPosition(vector_float2 newPos);
void Neon_UpdateDirectionalInput(vector_float2 newDir);
void Neon_UpdateMouseDown(bool down);

void Neon_UpdateNumberKeyPressed(int num);
void Neon_EscapePressed();

void Neon_UpdateTextureSize(TextureType tex, TexSize size);

void Neon_Start();
FrameData Neon_Render(float aspectRatio);

#ifdef __cplusplus
}
#endif
