#pragma once

#include "./Engine/ShaderTypes.h"
#include "./Engine/FrameData.h"

#include "NeonConstants.h"

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

void Neon_UpdateCursorPosition(float x, float y);
void Neon_UpdateDirectionalInput(float x, float y);
void Neon_UpdateMouseDown(bool down);

void Neon_UpdateNumberKeyPressed(int num);
void Neon_EscapePressed();

void Neon_UpdateTextureSize(TextureType tex, TexSize size);

void Neon_Start();
FrameData Neon_Render(float aspectRatio);

bool Neon_IsMusic(AudioType audio);

float Neon_SFXVolume();
float Neon_MusicVolume();

bool Neon_AppShouldQuit();

#ifdef __cplusplus
}
#endif
