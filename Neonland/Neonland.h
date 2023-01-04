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

void OnStart();
FrameData OnRender(float aspectRatio);

#ifdef __cplusplus
}
#endif
