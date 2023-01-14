#include "Mesh.hpp"

Mesh::Mesh(uint32_t idx) : _meshIdx{idx}, modelMatrix(1) {}

uint32_t Mesh::GetMeshIdx() const {
    return _meshIdx;
}
