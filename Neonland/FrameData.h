#pragma once

#include "ShaderTypes.h"

typedef struct FrameData {
    GlobalUniforms globalUniforms;
    
    size_t instanceCount;
    Instance* instances;
    
    size_t groupCount;
    size_t* groupSizes;
} FrameData;
