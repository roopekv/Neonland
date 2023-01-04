#pragma once

#include <simd/simd.h>

class Camera {
public:
    vector_float3 position;
    vector_float3 clearColor;
    float farPlane;
    float nearPlane;
    float verticalFoV;
    
    Camera(vector_float3 pos = {0, 0, -5}, vector_float3 color = {0, 0, 0}, float near = 0.1, float far = 500, float fov = 60);
};
