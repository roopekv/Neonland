#include "Physics.hpp"

Physics::Physics(float3 vel, float3 angularVel, float radius)
: velocity{vel}
, angularVelocity{angularVel}
, collisionRadius{radius} {}
