#include "Mesh.hpp"

Mesh::Mesh(Type type, TextureType tex, bool hidden)
: type{type}
, texture{tex}
, modelMatrix(1)
, hidden{hidden} {}
