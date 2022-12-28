#pragma once

#include <simd/simd.h>

struct VertexUniforms {
    simd_float3 offset;
    matrix_float4x4 projection;
};
