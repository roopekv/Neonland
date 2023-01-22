#include "EnemyType.hpp"

EnemyType::EnemyType(Enemy enemy, Mesh mesh, float3 scale, int maxHP)
: enemy(enemy)
, mesh(mesh)
, scale(scale)
, maxHP{maxHP} {}

const EnemyType& EnemyType::Swarm() {
    static const auto swarm = EnemyType(Enemy(6, 0.9f, 5, 4),
                                        Mesh(CUBE_MESH, Material(LIT_SHADER, NO_TEX, float4{0.24, 0.86, 0.59, 1})),
                                        float3{0.75, 0.75, 0.75},
                                        2);
    return swarm;
}
const EnemyType& EnemyType::Grunt() {
    static const auto grunt = EnemyType(Enemy(4, 0.5f, 3.5f, 1),
                                        Mesh(CUBE_MESH, Material(LIT_SHADER, NO_TEX, float4{0.97, 0.88, 0.01, 1})),
                                        float3{1, 1, 1},
                                        10);
    return grunt;
}
const EnemyType& EnemyType::Quick() {
    static const auto quick = EnemyType(Enemy(8, 0.9f, 7, 7),
                                        Mesh(CUBE_MESH, Material(LIT_SHADER, NO_TEX, float4{0.02, 0.66, 0.91, 1})),
                                        float3{1.5, 1.5, 1.5},
                                        10);
    return quick;
}
const EnemyType& EnemyType::Heavy() {
    static const auto heavy = EnemyType(Enemy(3, 0.9f, 2, 1, true),
                                        Mesh(CUBE_MESH, Material(LIT_SHADER, NO_TEX, float4{0.45, 0.11,  0.85, 1})),
                                        float3{3, 3, 3},
                                        50);
    return heavy;
}
const EnemyType& EnemyType::UberHeavy() {
    static const auto uberHeavy = EnemyType(Enemy(3, 0.9f, 2, 1, true),
                                            Mesh(CUBE_MESH, Material(LIT_SHADER, NO_TEX, float4{0.6, 0.1, 0.6, 1})),
                                            float3{6, 6, 6},
                                            150);
    return uberHeavy;
}
