#pragma once

#ifdef _WIN64
#include <DirectXMath.h>

using namespace DirectX;

using float2 = XMFLOAT2;
using float3 = XMFLOAT3;
using float4 = XMFLOAT4;

using float4x4 = XMFLOAT4X4;

float2 operator*(const float2& lhs, const float2& rhs);
float2 operator*(const float2& lhs, float rhs);
float2 operator*(float lhs, const float2& rhs);
float2 operator+(const float2& lhs, const float2& rhs);
float2 operator-(const float2& lhs, const float2& rhs);
float2& operator+=(float2& lhs, const float2& rhs);
float2& operator-=(float2& lhs, const float2& rhs);
float2& operator*=(float2& lhs, const float2& rhs);
float2& operator*=(float2& lhs, float rhs);

float2 operator/(const float2& lhs, float rhs);
float2& operator/=(float2& lhs, float rhs);

float3 operator*(const float3& lhs, const float3& rhs);
float3 operator*(const float3& lhs, float rhs);
float3 operator*(float lhs, const float3& rhs);
float3 operator+(const float3& lhs, const float3& rhs);
float3 operator-(const float3& lhs, const float3& rhs);
float3& operator+=(float3& lhs, const float3& rhs);
float3& operator-=(float3& lhs, const float3& rhs);
float3& operator*=(float3& lhs, const float3& rhs);
float3& operator*=(float3& lhs, float rhs);

float3 operator/(const float3& lhs, float rhs);
float3& operator/=(float3& lhs, float rhs);

float4 operator*(const float4& lhs, const float4& rhs);
float4 operator*(const float4& lhs, float rhs);
float4 operator*(float lhs, const float4& rhs);
float4 operator+(const float4& lhs, const float4& rhs);
float4 operator-(const float4& lhs, const float4& rhs);
float4& operator+=(float4& lhs, const float4& rhs);
float4& operator-=(float4& lhs, const float4& rhs);
float4& operator*=(float4& lhs, const float4& rhs);
float4& operator*=(float4& lhs, float rhs);

float4 operator/(const float4& lhs, float rhs);
float4& operator/=(float4& lhs, float rhs);

float4x4 operator*(const float4x4& lhs, const float4x4& rhs);
float4 operator*(const float4x4& lhs, const float4& rhs);

#elif __APPLE__
#include <simd/simd.h>
using float2 = simd::float2;
using float3 = simd::float3;
using float4 = simd::float4;

using float4x4 = simd::float4x4;
#endif

#include <numbers>
#include <cmath>
#include <ostream>

constexpr float DegToRad = std::numbers::pi_v<float> / 180;
constexpr float RadToDeg = 180 / std::numbers::pi_v<float>;

constexpr auto xAxis = float3{1, 0, 0};
constexpr auto yAxis = float3{0, 1, 0};
constexpr auto zAxis = float3{0, 0, 1};

float4x4 ProjectionMatrix(float verticalFoVInDegrees,
                          float aspectRatio,
                          float near,
                          float far);

float4x4 TranslationMatrix(float3 t);

float4x4 RotationMatrix(float3 axis, float degrees);

float4x4 ScaleMatrix(float3 s);

float3 VecNormalize(float3 vec);
float2 VecNormalize(float2 vec);

float VecLength(const float3& vec);
float VecLength(const float2& vec);

float4x4 InverseMatrix(float4x4 m);

std::ostream& operator<<(std::ostream &os, const float3& v);
std::ostream& operator<<(std::ostream &os, const float2& v);
std::ostream& operator<<(std::ostream &os, const float4& v);
std::ostream& operator<<(std::ostream &os, const float4x4& m);
