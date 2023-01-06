#pragma once

#include <simd/simd.h>

class Camera {
public:
    vector_float3 clearColor;
    
    Camera(vector_float3 pos = {0, 0, 0},
           vector_float3 color = {0, 0, 0},
           float near = 0.1f,
           float far = 100,
           float fov = 60,
           float aspectRatio = 1.0f);
    
    vector_float3 ScreenPointToWorld(vector_float2 screenPoint, float depth);
    
    void SetPosition(vector_float3 pos);
    const vector_float3& GetPosition() const;
    
    void SetClearColor(vector_float3 color);
    const vector_float3& GetClearColor() const;
    
    void SetFarClipPlane(float far);
    float GetFarClipPlane() const;
    
    void SetNearClipPlane(float near);
    float GetNearClipPlane() const;
    
    void SetVerticalFoV(float fov);
    float GetVerticalFoV() const;
    
    void SetAspectRatio(float ratio);
    float GetAspectRatio() const;
    
    const matrix_float4x4& GetProjectionMatrix();
    const matrix_float4x4& GetViewMatrix();
private:
    vector_float3 _position;
    float _farClipPlane;
    float _nearClipPlane;
    float _verticalFoV;
    float _aspectRatio;
    
    bool _projChanged;
    bool _viewChanged;
    
    matrix_float4x4 _projMat;
    matrix_float4x4 _viewMat;
};
