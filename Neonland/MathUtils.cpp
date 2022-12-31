#include "MathUtils.h"
#include <numbers>
#include <cmath>

matrix_float4x4 ProjectionMatrix(float verticalFoVInDegrees,
                                 float aspectRatio,
                                 float near,
                                 float far) {
    float fov = verticalFoVInDegrees * std::numbers::pi_v<float> / 180;
    
    float scale = 1 / std::tan(fov / 2);
    float x = scale / aspectRatio;
    float y = scale;
    
    float zRange = far - near;
    
    float z = -(far + near) / zRange;
    float w = -2 * far * near / zRange;
    
    matrix_float4x4 result;
    
    result.columns[0] = {x, 0, 0,  0};
    result.columns[1] = {0, y, 0,  0};
    result.columns[2] = {0, 0, z, -1};
    result.columns[3] = {0, 0, w,  0};
    
    return result;
}

matrix_float4x4 TranslationMatrix(vector_float3 t) {
    matrix_float4x4 result = matrix_identity_float4x4;
    result.columns[3] = {t.x, t.y, t.z,  1};
    
    return result;
}
