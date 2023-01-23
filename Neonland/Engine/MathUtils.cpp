#include "MathUtils.hpp"


float4x4 ProjectionMatrix(float verticalFoVInDegrees,
	float aspectRatio,
	float near,
	float far) {

	float fov = verticalFoVInDegrees * DegToRad;
#ifdef _WIN64
	float4x4 m;
	XMStoreFloat4x4(&m, XMMatrixPerspectiveFovLH(fov, aspectRatio, near, far));
	return m;
#elif __APPLE__

	float scale = 1 / std::tan(fov / 2);
	float x = scale / aspectRatio;
	float y = scale;

	float z = far / (far - near);
	float w = -near * far / (far - near);

	float4x4 m;

	m.columns[0] = { x, 0, 0,  0 };
	m.columns[1] = { 0, y, 0,  0 };
	m.columns[2] = { 0, 0, z,  1 };
	m.columns[3] = { 0, 0, w,  0 };
	return m;
#endif
}

float4x4 TranslationMatrix(float3 t) {
#ifdef _WIN64
	float4x4 m;
	XMStoreFloat4x4(&m, XMMatrixTranslation(t.x, t.y, t.z));
	return m;
#elif __APPLE__
	float4x4 result = matrix_identity_float4x4;
	result.columns[3] = { t.x, t.y, t.z,  1 };
	return result;
#endif
}

float4x4 RotationMatrix(float3 axis, float degrees) {
	float angle = degrees * DegToRad;
#ifdef _WIN64
	float4x4 m;
	XMStoreFloat4x4(&m, XMMatrixRotationAxis(XMLoadFloat3(&axis), angle));
	return m;
#elif __APPLE__
	float x = axis.x;
	float y = axis.y;
	float z = axis.z;

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
	m.columns[3] = { 0, 0, 0,  1 };

	return m;
#endif
}

float4x4 ScaleMatrix(float3 s) {
#ifdef _WIN64
	float4x4 m;
	XMStoreFloat4x4(&m, XMMatrixScaling(s.x, s.y, s.z));
	return m;
#elif __APPLE__
	return float4x4(float4{ s.x, s.y, s.z, 1 });
#endif
}

float3 VecNormalize(float3 vec) {
#ifdef _WIN64
	XMStoreFloat3(&vec, XMVector3Normalize(XMLoadFloat3(&vec)));
	return vec;
#elif __APPLE__
	float len = simd_length(vec);
	if (len > 0.0f) {
		vec /= len;
	}
	return vec;
#endif
}
float2 VecNormalize(float2 vec) {
#ifdef _WIN64
	XMStoreFloat2(&vec, XMVector2Normalize(XMLoadFloat2(&vec)));
	return vec;
#elif __APPLE__
	float len = simd_length(vec);
	if (len > 0.0f) {
		vec /= len;
	}
	return vec;
#endif
}

float VecLength(const float3& vec) {
#ifdef _WIN64
	float len;
	XMStoreFloat(&len, XMVector3Length(XMLoadFloat3(&vec)));
	return len;
#elif __APPLE__
	return simd_length(vec);
#endif
}

float VecLength(const float2& vec) {
#ifdef _WIN64
	float len;
	XMStoreFloat(&len, XMVector2Length(XMLoadFloat2(&vec)));
	return len;
#elif __APPLE__
	return simd_length(vec);
#endif
}

float4x4 InverseMatrix(float4x4 m) {
#ifdef _WIN64
	XMStoreFloat4x4(&m, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m)));
	return m;
#elif __APPLE__
	return simd::inverse(m);
#endif
}

std::ostream& operator<<(std::ostream& os, const float3& v) {
	return os << v.x << " " << v.y << " " << v.z;
}

std::ostream& operator<<(std::ostream& os, const float2& v) {
	return os << v.x << " " << v.y;
}

std::ostream& operator<<(std::ostream& os, const float4& v) {
	return os << v.x << " " << v.y << " " << v.z << " " << v.w;
}

std::ostream& operator<<(std::ostream& os, const float4x4& m) {
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
#ifdef _WIN64
			os << m.m[x][y] << " ";
#elif __APPLE__
			os << m.columns[x][y] << " ";
#endif
		}
		os << std::endl;
	}
	return os;
}
