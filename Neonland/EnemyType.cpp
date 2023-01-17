#include "EnemyType.hpp"

EnemyType::EnemyType(Enemy enemy, Mesh mesh, float3 scale, int maxHP)
: enemy(enemy)
, mesh(mesh)
, scale(scale)
, maxHP{maxHP} {}

const EnemyType& EnemyType::Swarm() {
    static const auto swarm = EnemyType(Enemy(6, 0.9f, 2), Mesh(CUBE_MESH, NO_TEX, float4{0.24, 0.86, 0.59, 1}), float3{0.75, 0.75, 0.75}, 1);
    return swarm;
}
const EnemyType& EnemyType::Grunt() {
    static const auto grunt = EnemyType(Enemy(4, 0.5f, 1), Mesh(CUBE_MESH, NO_TEX, float4{1.0, 0.37, 0.33, 1}), float3{1, 1, 1}, 10);
    return grunt;
}
const EnemyType& EnemyType::Quick() {
    static const auto quick = EnemyType(Enemy(8, 0.9f, 3), Mesh(CUBE_MESH, NO_TEX, float4{0.02, 0.66, 0.91, 1}), float3{1.5, 1.5, 1.5}, 10);
    return quick;
}
const EnemyType& EnemyType::Heavy() {
    static const auto heavy = EnemyType(Enemy(3, 0.9f, 1), Mesh(CUBE_MESH, NO_TEX, float4{0.45, 0.11,  0.85, 1}), float3{4, 4, 4}, 100);
    return heavy;
}
const EnemyType& EnemyType::UberHeavy() {
    static const auto uberHeavy = EnemyType(Enemy(3, 0.9f, 1), Mesh(CUBE_MESH, NO_TEX, float4{0.89, 0.78, 0.27, 1}), float3{8, 8, 8}, 200);
    return uberHeavy;
}
