#include "Mesh.hpp"

Mesh::Mesh(MeshType type, Material material, bool hidden, float4 tint)
: type{type}
, material(material)
, hidden{hidden}
, tint{tint} {}
