#pragma once

#include "Enemy.hpp"
#include "Mesh.hpp"

class EnemyType {
public:
    static const EnemyType& Swarm();
    static const EnemyType& Grunt();
    static const EnemyType& Quick();
    static const EnemyType& Heavy();
    static const EnemyType& UberHeavy();
    
    const Enemy enemy;
    const Mesh mesh;
    const float3 scale;
    const int maxHP;
    
private:
    EnemyType(Enemy enemy, Mesh mesh, float3 scale, int maxHP);
};
