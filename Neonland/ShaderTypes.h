#pragma once

#include <simd/simd.h>

typedef struct GlobalUniforms {
    matrix_float4x4 viewMatrix;
    matrix_float4x4 projMatrix;
} GlobalUniforms;

typedef struct Instance {
    matrix_float4x4 transform;
} Instance;
