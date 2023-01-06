#include "Camera.hpp"
#include "MathUtils.h"

#include <iostream>

Camera::Camera(vector_float3 pos, vector_float3 color, float near, float far, float fov, float aspectRatio)
: _position{pos}
, clearColor{color}
, _nearClipPlane{near}
, _farClipPlane{far}
, _verticalFoV{fov}
, _aspectRatio{aspectRatio}
, _projChanged{true}
, _viewChanged{true}{}

vector_float3 Camera::ScreenPointToWorld(vector_float2 screenPoint, float depth) {
    simd::float4x4 proj = ProjectionMatrix(_verticalFoV, _aspectRatio, _nearClipPlane, _farClipPlane);
    simd::float4x4 view = TranslationMatrix(-_position);
    
    simd::float4 worldSpaceDepth = {0, 0, depth, 1};
    
    simd::float4 screenSpaceDepth = proj * view * worldSpaceDepth;
    screenSpaceDepth /= screenSpaceDepth.w;
    
    simd::float4x4 inverse = simd::inverse(proj * view);
    
    vector_float4 worldPoint = inverse * simd::float4{screenPoint.x, screenPoint.y, screenSpaceDepth.z, 1};
    worldPoint /= worldPoint.w;
    return {worldPoint.x, worldPoint.y, worldPoint.z};
}

void Camera::SetPosition(vector_float3 pos) {
    _viewChanged = true;
    _position = pos;
}

const vector_float3& Camera::GetPosition() const {
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

const matrix_float4x4& Camera::GetProjectionMatrix() {
    if (_projChanged) {
        _projMat = ProjectionMatrix(_verticalFoV, _aspectRatio, _nearClipPlane, _farClipPlane);
        _projChanged = false;
    }
    
    return _projMat;
}

const matrix_float4x4& Camera::GetViewMatrix() {
    if (_viewChanged) {
        _viewMat = TranslationMatrix(-_position);
        _viewChanged = false;
    }
    
    return _viewMat;
}
