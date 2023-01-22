#include "Level.hpp"

Level::Level(std::vector<Wave> waves, TextureType ground, float2 mapSize)
: waves(waves)
, groundTexture{ground}
, mapSize{mapSize} {}

const Level& Level::GetLevel(int i) {
    static const std::array<Level, 3> Levels = {
        Level({
            Wave({
                {EnemyType::Grunt(), 5, 5.0},
                
                {EnemyType::Grunt(), 5, 5.0},
                
                {EnemyType::Grunt(), 10, 0},
                {EnemyType::Swarm(), 10, 5.0},
                
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                
                {EnemyType::Swarm(), 10, 0.0},
                {EnemyType::Grunt(), 10, 5.0},
                {EnemyType::Swarm(), 30, 5.0},
            }),
            Wave({
                {EnemyType::Grunt(), 10, 0.0},
                {EnemyType::Swarm(), 30, 10.0},
                
                {EnemyType::Grunt(), 10, 0.0},
                {EnemyType::Swarm(), 30, 10.0},
                
                {EnemyType::Swarm(), 20, 5.0},
                
                {EnemyType::Swarm(), 20, 5.0},
                
                {EnemyType::Swarm(), 10, 5.0},
                
                {EnemyType::Grunt(), 10, 5.0},
                
                {EnemyType::Swarm(), 20, 2.5},
                
                {EnemyType::Swarm(), 20, 2.5},
                
                {EnemyType::Swarm(), 10, 2.5},
                
                {EnemyType::Swarm(), 10, 2.5},
                
                {EnemyType::Swarm(), 5, 1},
                {EnemyType::Swarm(), 5, 1},
                {EnemyType::Swarm(), 10, 1},
                {EnemyType::Swarm(), 10, 1},
            }),
            Wave({
                {EnemyType::Grunt(), 50, 0.0},
                {EnemyType::Swarm(), 20, 15.0},
                
                {EnemyType::Swarm(), 50, 0.0},
                {EnemyType::Heavy(), 1, 5.0},
                
                {EnemyType::Swarm(), 15, 5.0},
                {EnemyType::Swarm(), 30, 5.0},
                {EnemyType::Swarm(), 60, 5.0},
                
                {EnemyType::Swarm(), 30, 0.0},
                {EnemyType::Heavy(), 3, 15.0},
                
                {EnemyType::Swarm(), 30, 5.0},
                
                {EnemyType::Swarm(), 30, 2.5},
                
                {EnemyType::Swarm(), 30, 2.5},
                
                {EnemyType::Swarm(), 5, 1},
                {EnemyType::Swarm(), 5, 1},
                {EnemyType::Swarm(), 10, 1},
                {EnemyType::Swarm(), 10, 1},
            })
        }, GROUND1_TEX, float2{60, 60}),
        Level({
            Wave({
                {EnemyType::Quick(), 1,  0.0},
                {EnemyType::Swarm(), 20, 10.0},
                {EnemyType::Quick(), 1,  0.0},
                {EnemyType::Swarm(), 10, 2.5},
                {EnemyType::Quick(), 1,  0.0},
                {EnemyType::Swarm(), 10, 2.5},
                {EnemyType::Quick(), 1,  0.0},
                {EnemyType::Swarm(), 10, 2.5},
                {EnemyType::Quick(), 1,  0.0},
                {EnemyType::Swarm(), 10, 2.5},
                {EnemyType::Quick(), 5, 2.5},
                {EnemyType::Swarm(), 10, 2.5},
                {EnemyType::Swarm(), 20, 2.5},
                {EnemyType::Swarm(), 20, 2.5},
                {EnemyType::Swarm(), 20, 2.5},
                {EnemyType::Swarm(), 20, 2.5}
            }),
            Wave({
                {EnemyType::Grunt(), 20, 0},
                {EnemyType::Heavy(), 3, 0},
                
                {EnemyType::Swarm(), 10, 2.5f},
                {EnemyType::Swarm(), 10, 2.5f},
                {EnemyType::Swarm(), 10, 2.5f},
                {EnemyType::Swarm(), 20, 2.5f},
                {EnemyType::Swarm(), 20, 2.5f},
                {EnemyType::Swarm(), 20, 2.5f},
                
                {EnemyType::Heavy(), 5, 0.0},
                {EnemyType::Grunt(), 10, 10.0},
                {EnemyType::Swarm(), 10, 2.5f},
                {EnemyType::Swarm(), 10, 2.5f},
                {EnemyType::Swarm(), 10, 2.5f},
                {EnemyType::Swarm(), 10, 2.5f},
                {EnemyType::Swarm(), 10, 2.5f}
            }),
            Wave({
                {EnemyType::Quick(), 5, 5.0},
                {EnemyType::Quick(), 5, 5.0},
                {EnemyType::Quick(), 5, 5.0},
                {EnemyType::Quick(), 5, 5.0},
                {EnemyType::Quick(), 5, 5.0},
                {EnemyType::Heavy(), 1, 5.0},
                {EnemyType::Swarm(), 20, 5.0},
                {EnemyType::Swarm(), 20, 5.0},
                {EnemyType::Swarm(), 20, 10.0},
                {EnemyType::Quick(), 5, 5.0},
                {EnemyType::Quick(), 6, 5.0},
                {EnemyType::Quick(), 7, 5.0},
                {EnemyType::Quick(), 8, 5.0},
                {EnemyType::Quick(), 9, 5.0}
            })
        }, GROUND2_TEX, float2{75, 75}),
        Level({
            Wave({
                {EnemyType::UberHeavy(), 1, 0}
            }),
            Wave({
                {EnemyType::UberHeavy(), 1, 0},
                {EnemyType::Swarm(), 5, 5.0},
                {EnemyType::Swarm(), 5, 2.5},
                {EnemyType::Swarm(), 5, 1.25},
                {EnemyType::Swarm(), 5, 0.625},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Swarm(), 10, 2.5},
                {EnemyType::Swarm(), 10, 1.25},
                {EnemyType::Swarm(), 10, 0.625},
                {EnemyType::Swarm(), 15, 5.0},
                {EnemyType::Swarm(), 15, 2.5},
                {EnemyType::Swarm(), 15, 1.25},
                {EnemyType::Swarm(), 15, 0.625}
            }),
            Wave({
                {EnemyType::Heavy(), 3, 0},
                {EnemyType::Grunt(), 10, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Quick(), 1, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Quick(), 1, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Swarm(), 10, 5.0}
            }),
            Wave({
                {EnemyType::UberHeavy(), 3, 0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Quick(), 1, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Quick(), 1, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Quick(), 1, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Quick(), 1, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Quick(), 1, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Quick(), 1, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Quick(), 1, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Quick(), 1, 5.0},
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::Quick(), 1, 5.0},
                {EnemyType::Swarm(), 10, 5.0}
            }),
            Wave({
                {EnemyType::Swarm(), 10, 5.0},
                {EnemyType::UberHeavy(), 5, 0},
                {EnemyType::Quick(), 1, 5.0},
                {EnemyType::Swarm(), 15, 5.0},
                {EnemyType::Quick(), 1, 5.0},
                {EnemyType::Swarm(), 15, 5.0},
                {EnemyType::Swarm(), 15, 2.5},
                {EnemyType::Quick(), 1, 2.5},
                {EnemyType::Swarm(), 15, 2.5},
                {EnemyType::Quick(), 1, 2.5},
                {EnemyType::Swarm(), 20, 2.5},
                {EnemyType::Swarm(), 20, 2.5},
                {EnemyType::Quick(), 1, 2.5},
                {EnemyType::Swarm(), 20, 2.5},
                {EnemyType::Quick(), 1, 2.5},
                {EnemyType::Swarm(), 20, 2.5}
            })
        }, GROUND3_TEX, float2{20, 20})
    };
    
    return Levels[i];
}
