#include "Material.hpp"

Material::Material(ShaderType shader, TextureType tex, float4 color)
: shader{shader}
, texture{tex}
, color{color} {}
