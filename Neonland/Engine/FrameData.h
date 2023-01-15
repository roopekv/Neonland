#pragma once

#include "ShaderTypes.h"

typedef struct FrameData {
    GlobalUniforms globalUniforms;
    vector_float3 clearColor;
    
    size_t instanceCount;
    Instance* instances;
    
    uint32_t groupCount;
    
    size_t* groupSizes;
    uint32_t* groupMeshes;
    uint32_t* groupTextures;
} FrameData;
