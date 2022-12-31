#pragma once

#include "ShaderTypes.h"

typedef struct {
    GlobalUniforms globalUniforms;
    size_t instanceCount;
    Instance* instances;
} FrameData;
