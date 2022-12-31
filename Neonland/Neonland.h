#pragma once

#include <stddef.h>
#include <stdint.h>

#include "ShaderTypes.h"
#include "FrameData.h"
#include "MathUtils.h"

const double TIMESTEP = 1.0 / 1;

const size_t MAX_ENEMY_COUNT = 100;
const size_t MAX_PROJECTILE_COUNT = 0;
const size_t MAX_ENTITY_COUNT = MAX_ENEMY_COUNT + MAX_PROJECTILE_COUNT + 1;

#ifdef __cplusplus
extern "C" {
#endif

void OnStart();
FrameData OnRender(float aspectRatio);

#ifdef __cplusplus
}
#endif
