#pragma once

#include "NeonConstants.h"
#include "MathUtils.hpp"

class Material {
public:
    bool operator<(const Material& rhs) const {
        return shader < rhs.shader || (shader == rhs.shader && texture < rhs.texture);
    }
    
    bool operator>(const Material& rhs) const {
        return shader > rhs.shader || (shader == rhs.shader && texture > rhs.texture);
    }
    
    bool operator==(const Material& rhs) const {
        return shader == rhs.shader && texture == rhs.texture;
    }
    
    ShaderType shader;
    TextureType texture;
    float4 color;
    
    Material(ShaderType shader, TextureType tex = NO_TEX, float4 color = {1, 1, 1, 1});
};
