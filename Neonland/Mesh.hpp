#pragma once

#include "MathUtils.hpp"
#include "ComponentType.hpp"

class Mesh {
public:
    static constexpr ComponentType componentType = ComponentType::mesh;
    bool operator<(const Mesh& rhs) const {
        return _meshIdx < rhs._meshIdx;
    }
    
    bool operator>(const Mesh& rhs) const {
        return _meshIdx > rhs._meshIdx;
    }
    
    bool operator==(const Mesh& rhs) const {
        return _meshIdx == rhs._meshIdx;
    }
    
    float4x4 modelMatrix;
    uint32_t GetMeshIdx() const;
    
    Mesh(uint32_t idx);
private:
    uint32_t _meshIdx;
};
