#pragma once

#include <simd/simd.h>

typedef struct {
    matrix_float4x4 viewMatrix;
    matrix_float4x4 projMatrix;
} GlobalUniforms;

typedef struct {
    matrix_float4x4 transform;
    int32_t meshIndex;
} Instance;
