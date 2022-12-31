#pragma once

#include <simd/simd.h>

#ifdef __cplusplus
extern "C" {
#endif

matrix_float4x4 ProjectionMatrix(float verticalFoVInDegrees,
                                 float aspectRatio,
                                 float near,
                                 float far);

matrix_float4x4 TranslationMatrix(vector_float3 t);

#ifdef __cplusplus
}
#endif
