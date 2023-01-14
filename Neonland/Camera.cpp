#include "Camera.hpp"

#include <iostream>

Camera::Camera(float3 pos, float3 color, float near, float far, float fov, float aspectRatio)
: _position{pos}
, clearColor{color}
, _nearClipPlane{near}
, _farClipPlane{far}
, _verticalFoV{fov}
, _aspectRatio{aspectRatio}
, _projChanged{true}
, _viewChanged{true}{}

vector_float3 Camera::ScreenPointToWorld(float2 screenPoint, float depth) {
    float4x4 proj = ProjectionMatrix(_verticalFoV, _aspectRatio, _nearClipPlane, _farClipPlane);
    float4x4 view = TranslationMatrix(-_position);
    
    float4 worldSpaceDepth = {0, 0, depth, 1};
    
    float4 screenSpaceDepth = proj * view * worldSpaceDepth;
    screenSpaceDepth /= screenSpaceDepth.w;
    
    float4x4 inverse = simd::inverse(proj * view);
    
    float4 worldPoint = inverse * float4{screenPoint.x, screenPoint.y, screenSpaceDepth.z, 1};
    worldPoint /= worldPoint.w;
    return {worldPoint.x, worldPoint.y, worldPoint.z};
}

void Camera::SetPosition(float3 pos) {
    _viewChanged = true;
    _position = pos;
}

const float3& Camera::GetPosition() const {
    return _position;
}

void Camera::SetFarClipPlane(float far) {
    _projChanged = true;
    _farClipPlane = far;
}

float Camera::GetFarClipPlane() const {
    return _farClipPlane;
}

void Camera::SetNearClipPlane(float near) {
    _projChanged = true;
    _nearClipPlane = near;
}

float Camera::GetNearClipPlane() const {
    return _nearClipPlane;
}

void Camera::SetVerticalFoV(float fov) {
    _projChanged = true;
    _verticalFoV = fov;
}

float Camera::GetVerticalFoV() const {
    return _verticalFoV;
}

void Camera::SetAspectRatio(float ratio) {
    _projChanged = true;
    _aspectRatio = ratio;
}

float Camera::GetAspectRatio() const {
    return _aspectRatio;
}

const float4x4& Camera::GetProjectionMatrix() {
    if (_projChanged) {
        _projMat = ProjectionMatrix(_verticalFoV, _aspectRatio, _nearClipPlane, _farClipPlane);
        _projChanged = false;
    }
    
    return _projMat;
}

const float4x4& Camera::GetViewMatrix() {
    if (_viewChanged) {
        _viewMat = TranslationMatrix(-_position);
        _viewChanged = false;
    }
    
    return _viewMat;
}
