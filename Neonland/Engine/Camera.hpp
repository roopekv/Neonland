#pragma once

#include "MathUtils.hpp"

class Camera {
public:
    float3 clearColor;
    
    Camera(float3 pos = {0, 0, 0},
           float3 color = {0, 0, 0},
           float near = 0.1f,
           float far = 100,
           float fov = 60,
           float aspectRatio = 1.0f);
    
    float3 ScreenPointToWorld(float2 screenPoint, float depth);
    
    void SetPosition(float3 pos);
    const float3& GetPosition() const;
    
    void SetClearColor(float3 color);
    const float3& GetClearColor() const;
    
    void SetFarClipPlane(float far);
    float GetFarClipPlane() const;
    
    void SetNearClipPlane(float near);
    float GetNearClipPlane() const;
    
    void SetVerticalFoV(float fov);
    float GetVerticalFoV() const;
    
    void SetAspectRatio(float ratio);
    float GetAspectRatio() const;
    
    const float4x4& GetProjectionMatrix();
    const float4x4& GetViewMatrix();
private:
    float3 _position;
    float _farClipPlane;
    float _nearClipPlane;
    float _verticalFoV;
    float _aspectRatio;
    
    bool _projChanged;
    bool _viewChanged;
    
    float4x4 _projMat;
    float4x4 _viewMat;
};
