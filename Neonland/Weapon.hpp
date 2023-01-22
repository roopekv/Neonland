#pragma once

#include "PlayerProjectile.hpp"
#include "Mesh.hpp"

class Weapon {
public:
    const PlayerProjectile projectile;
    const Mesh projectileMesh;
    const double cooldown;
    
    const float spread;
    const int projectilesPerShot;
    const float projectileSize;
    
    const AudioType audio;
    
    double cooldownEndTime;
    
    Weapon(PlayerProjectile projectile, double cooldown, float spread, int projectilesPerShot, float projectileSize, Mesh projectileMesh, AudioType audio);
};
