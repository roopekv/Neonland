#pragma once

#include "../Engine/MathUtils.hpp"
#include "../Engine/ComponentType.hpp"

class Camera {
public:
    static constexpr ComponentType componentType = ComponentType::camera;
    bool operator<(const Camera& rhs) const {
        return false;
    }
    
    bool operator>(const Camera& rhs) const {
        return false;
    }
    
    bool operator==(const Camera& rhs) const {
        return true;
    }
    
    float3 clearColor;
    
    Camera(float3 pos = {0, 0, 0},
           float3 rot = {0, 0, 0},
           float3 color = {0, 0, 0},
           float near = 0.1f,
           float far = 100,
           float fov = 60,
           float aspectRatio = 1.0f);
    
    float3 ScreenPointToWorld(float2 screenPoint, float depth);
    
    float3 Forward();
    
    void SetPosition(float3 pos);
    const float3& GetPosition() const;
    
    void SetRotation(float3 rot);
    const float3& GetRotation() const;
    
    void SetFarClipPlane(float far);
    float GetFarClipPlane() const;
    
    void SetNearClipPlane(float near);
    float GetNearClipPlane() const;
    
    void SetVerticalFoV(float fov);
    float GetVerticalFoV() const;
    
    void SetAspectRatio(float ratio);
    float GetAspectRatio() const;
    
    float4x4 GetProjectionMatrix();
    float4x4 GetTranslationMatrix();
    float4x4 GetRotationMatrix();
    float4x4 GetViewMatrix();
private:
    float3 _position;
    float3 _rotation;
    float _farClipPlane;
    float _nearClipPlane;
    float _verticalFoV;
    float _aspectRatio;
    
    bool _projChanged;
    bool _positionChanged;
    bool _rotationChanged;
    
    float4x4 _projMat;
    float4x4 _translationMatrix;
    float4x4 _rotationMatrix;
};
