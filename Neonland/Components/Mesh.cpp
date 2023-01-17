#include "Mesh.hpp"

Mesh::Mesh(MeshType type, TextureType tex, float4 color, bool hidden, float colorMult)
: type{type}
, texture{tex}
, modelMatrix(1)
, color{color}
, hidden{hidden}
, colorMult{colorMult} {}
