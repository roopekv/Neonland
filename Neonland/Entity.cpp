#include "Entity.hpp"

Entity::Entity(vector_float3 pos, float rot, vector_float3 vel, float angularVel, int32_t meshIdx)
: _position(pos)
, _rotation(rot)
, vel(vel)
, angularVel(angularVel)
, meshIdx(meshIdx)
, _movedOutsideUpdate(false)
, _rotatedOutsideUpdate(false) {}

void Entity::Update(double timestep) {
    _position += vel * timestep;
    _movedOutsideUpdate = false;
    _rotatedOutsideUpdate = false;
}

vector_float3& Entity::Position() {
    _movedOutsideUpdate = true;
    return _position;
}

float& Entity::Rotation() {
    _rotatedOutsideUpdate = true;
    return _rotation;
}

const vector_float3& Entity::Position() const { return _position; };
const float& Entity::Rotation() const { return _rotation; };

matrix_float4x4 Entity::Transform(double timeSinceUpdate) const {
    if (_movedOutsideUpdate) {
        return TranslationMatrix(_position);
    }
    return TranslationMatrix(_position + vel * timeSinceUpdate);
}

Instance Entity::Instance(double timeSinceUpdate) const {
    return {Transform(timeSinceUpdate), meshIdx};;
}
