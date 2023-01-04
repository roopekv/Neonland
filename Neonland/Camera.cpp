#include "Camera.hpp"

Camera::Camera(vector_float3 pos, vector_float3 color, float near, float far, float fov)
: position{pos}
, clearColor{color}
, nearPlane{near}
, farPlane{far}
, verticalFoV{fov} {}
