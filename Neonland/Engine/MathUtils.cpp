#include "MathUtils.hpp"
#include <numbers>
#include <cmath>

namespace {
constexpr float degToRad = std::numbers::pi_v<float> / 180;
}

float4x4 ProjectionMatrix(float verticalFoVInDegrees,
                          float aspectRatio,
                          float near,
                          float far) {
    float fov = verticalFoVInDegrees * degToRad;
    
    float scale = 1 / std::tan(fov / 2);
    float x = scale / aspectRatio;
    float y = scale;
    
    float z = far / (far - near);
    float w = -near * far / (far - near);
    
    float4x4 m;
    
    m.columns[0] = {x, 0, 0,  0};
    m.columns[1] = {0, y, 0,  0};
    m.columns[2] = {0, 0, z,  1};
    m.columns[3] = {0, 0, w,  0};
    
    return m;
}

float4x4 TranslationMatrix(float3 t) {
    float4x4 result = matrix_identity_float4x4;
    result.columns[3] = {t.x, t.y, t.z,  1};
    
    return result;
}

float4x4 RotationMatrix(float3 axis, float degrees) {
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    
    float angle = degrees * degToRad;
    float s = std::sin(angle);
    float c = std::cos(angle);
    
    float4x4 m;
    
    m.columns[0] = {
        c + x * x * (1 - c),
        y * x * (1 - c) + z * s,
        z * x * (1 - c) - y * s,
        0
    };
    
    m.columns[1] = {
        x * y * (1 - c) - z * s,
        c + y * y * (1 - c),
        z * y * (1 - c) + x * s,
        0
    };
    m.columns[2] = {
        x * z * (1 - c) + y * s,
        y * z * (1 - c) - x * s,
        c + z * z * (1 - c),
        0
    };
    m.columns[3] = {0, 0, 0,  1};
    
    return m;
}

float4x4 ScaleMatrix(float3 s) {
    return float4x4(float4{s.x, s.y, s.z, 1});
}
