#pragma once

#ifdef _WIN64
#include <DirectXMath.h>

typedef struct GlobalUniforms {
    DirectX::XMFLOAT4X4 viewMatrix;
    DirectX::XMFLOAT4X4 projMatrix;
} GlobalUniforms;

typedef struct Instance {
    DirectX::XMFLOAT4X4 transform;
    DirectX::XMFLOAT4 color;
} Instance;

#elif __APPLE__
#include <simd/simd.h>

typedef struct GlobalUniforms {
    matrix_float4x4 viewMatrix;
    matrix_float4x4 projMatrix;
} GlobalUniforms;

typedef struct Instance {
    matrix_float4x4 transform;
    vector_float4 color;
} Instance;

#endif

