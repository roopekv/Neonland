#include "Wave.hpp"

namespace {

int GetEnemyCount(const std::vector<Wave::SubWave>& subWaves) {
    int count = 0;
    for (auto& subWave : subWaves) {
        count += subWave.count;
    }
    return count;
}

}

Wave::Wave(std::vector<SubWave> subWaves) : subWaves(subWaves), enemyCount{GetEnemyCount(subWaves)} {}

