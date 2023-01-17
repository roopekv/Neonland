#pragma once

#include <array>
#include "Wave.hpp"

class Level {
public:
    const std::vector<Wave> waves;
    const TextureType groundTexture;
    
    const float2 mapSize;
    
    static const Level& GetLevel(int i);
private:
    Level(std::vector<Wave> waves, TextureType ground, float2 mapSize);};
