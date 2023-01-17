#pragma once

#include <vector>
#include <tuple>
#include "EnemyType.hpp"

class Wave {
public:
    class SubWave {
    public:
        const EnemyType type;
        size_t count;
        double duration;
        SubWave(EnemyType type, size_t count, double duration)
        : type{type}
        , count{count}
        , duration{duration} {}
    };
    
    const std::vector<SubWave> subWaves;
    const int enemyCount;
    
    Wave(std::vector<SubWave> subWaves);
};
