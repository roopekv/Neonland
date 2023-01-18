#include "Level.hpp"

Level::Level(std::vector<Wave> waves, TextureType ground, float2 mapSize)
: waves(waves)
, groundTexture{ground}
, mapSize{mapSize} {}

const Level& Level::GetLevel(int i) {
    static const std::array<Level, 1> Levels = {
        Level({
            Wave({
                {EnemyType::Grunt(), 10, 0.0},
                {EnemyType::Swarm(), 10, 10.0},
                {EnemyType::Swarm(), 20, 0.0},
                {EnemyType::Grunt(), 25, 5.0},
                {EnemyType::Swarm(), 30, 5.0},
                {EnemyType::Swarm(), 30, 0.0},
                {EnemyType::Grunt(), 20, 5.0},
                {EnemyType::Swarm(), 10, 0.0},
            }),
            Wave({
                {EnemyType::Grunt(), 10, 0.0},
                {EnemyType::Swarm(), 30, 5.0},
                {EnemyType::Grunt(), 10, 0.0},
                {EnemyType::Swarm(), 60, 15.0},
                {EnemyType::Swarm(), 100, 15.0}
            }),
            Wave({
                {EnemyType::Grunt(), 50, 0.0},
                {EnemyType::Swarm(), 20, 15.0},
                {EnemyType::Swarm(), 50, 0.0},
                {EnemyType::Heavy(), 1, 15.0},
                {EnemyType::Swarm(), 100, 0.0},
                {EnemyType::Heavy(), 10, 15.0}
            })
        }, GROUND0_TEX, float2{50, 50}),
    };
    
    return Levels[i];
}
