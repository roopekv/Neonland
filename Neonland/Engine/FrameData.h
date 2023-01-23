#pragma once

#include "ShaderTypes.h"

typedef struct FrameData {
    GlobalUniforms globalUniforms;

#ifdef _WIN64
    DirectX::XMFLOAT3 clearColor;
#elif __APPLE__
    vector_float3 clearColor;
#endif

    size_t instanceCount;
    Instance* instances;
    
    uint32_t groupCount;
    
    size_t* groupSizes;
    uint32_t* groupMeshes;
    uint32_t* groupTextures;
    uint32_t* groupShaders;
    
    size_t audioCount;
    uint32_t* audios;
    
} FrameData;
