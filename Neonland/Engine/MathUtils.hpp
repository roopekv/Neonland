#pragma once

#include <simd/simd.h>
#include <numbers>
#include <cmath>

using float2 = simd::float2;
using float3 = simd::float3;
using float4 = simd::float4;

using float4x4 = simd::float4x4;

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

float VecDistance(const float3& a, const float3& b);
float VecDistance(const float2& a, const float2& b);
