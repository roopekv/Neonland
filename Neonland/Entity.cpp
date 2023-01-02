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
    _rotation += angularVel * timestep;
    _movedOutsideUpdate = false;
    _rotatedOutsideUpdate = false;
}

void Entity::SetPosition(vector_float3& pos) {
    _movedOutsideUpdate = true;
    _position = pos;
}

void Entity::SetRotation(float rot) {
    _rotatedOutsideUpdate = true;
    _rotation = rot;
}

const vector_float3& Entity::GetPosition() const { return _position; };
const float& Entity::GetRotation() const { return _rotation; };

matrix_float4x4 Entity::GetTransform(double timeSinceUpdate) const {
    constexpr static auto zAxis = vector_float3{0, 0, 1};
    
    if (_movedOutsideUpdate) {
        return matrix_multiply(TranslationMatrix(_position),
                               RotationMatrix(zAxis, _rotation));
    }
    return matrix_multiply(TranslationMatrix(_position + vel * timeSinceUpdate),
                           RotationMatrix(zAxis, _rotation + angularVel * timeSinceUpdate));
}

Instance Entity::GetInstance(double timeSinceUpdate) const {
    Instance instance;
    instance.transform = GetTransform(timeSinceUpdate);
    return instance;
}
