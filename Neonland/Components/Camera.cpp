#include "Camera.hpp"

#include <iostream>

Camera::Camera(float3 pos, float3 rot, float3 color, float near, float far, float fov, float aspectRatio)
: _position{pos}
, _rotation{rot}
, clearColor{color}
, _nearClipPlane{near}
, _farClipPlane{far}
, _verticalFoV{fov}
, _aspectRatio{aspectRatio}
, _projChanged{true}
, _positionChanged{true}
, _rotationChanged{true} {}

vector_float3 Camera::ScreenPointToWorld(float2 screenPoint, float depth) {
    float4x4 proj = GetProjectionMatrix();
    float4x4 view = GetViewMatrix();
    
    float4 worldSpaceDepth = {0, 0, depth, 1};
    
    float4 screenSpaceDepth = proj * view * worldSpaceDepth;
    screenSpaceDepth /= screenSpaceDepth.w;
    
    float4x4 inverse = simd::inverse(proj * view);
    
    float4 worldPoint = inverse * float4{screenPoint.x, screenPoint.y, screenSpaceDepth.z, 1};
    worldPoint /= worldPoint.w;
    
    return {worldPoint.x, worldPoint.y, worldPoint.z};
}

float3 Camera::Forward(){
    float4 forward = GetRotationMatrix() * float4{0, 0, 1, 1};
    return float3{forward.x, forward.y, forward.z};
}

void Camera::SetPosition(float3 pos) {
    _positionChanged = true;
    _position = pos;
}

//const float3 Camera::GetLocalPosition() {
//    float4 rPos = simd::inverse(GetRotationMatrix()) * float4{_position.x, _position.y, _position.z, 1};
//    return {rPos.x, rPos.y, rPos.z};
//}

const float3& Camera::GetPosition() const {
    return _position;
}

void Camera::SetRotation(float3 rot) {
    _rotationChanged = true;
    _rotation = rot;
}

const float3& Camera::GetRotation() const {
    return _rotation;
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
    if (ratio != _aspectRatio) {
        _projChanged = true;
        _aspectRatio = ratio;
    }
}

float Camera::GetAspectRatio() const {
    return _aspectRatio;
}

float4x4 Camera::GetProjectionMatrix() {
    if (_projChanged) {
        _projMat = ProjectionMatrix(_verticalFoV, _aspectRatio, _nearClipPlane, _farClipPlane);
        _projChanged = false;
    }
    
    return _projMat;
}

float4x4 Camera::GetTranslationMatrix() {
    if (_positionChanged) {
        _translationMatrix = TranslationMatrix(_position);
        _positionChanged = false;
    }
    
    return _translationMatrix;
}

float4x4 Camera::GetRotationMatrix() {
    if (_rotationChanged) {
        float4x4 rX = RotationMatrix(xAxis, _rotation.x);
        float4x4 rY = RotationMatrix(yAxis, _rotation.y);
        float4x4 rZ = RotationMatrix(zAxis, _rotation.z);
        _rotationMatrix = rZ * rY * rX;
        _rotationChanged = false;
    }
    
    return _rotationMatrix;
}

float4x4 Camera::GetViewMatrix() {
    return simd::inverse(GetTranslationMatrix() * GetRotationMatrix());
}
