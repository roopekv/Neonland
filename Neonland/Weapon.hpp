#pragma once

#include "PlayerProjectile.hpp"

class Weapon {
public:
    const PlayerProjectile bullet;
    const double cooldown;
    
    const float spread;
    const int bulletsPerShot;
    const float bulletSize;
    
    double cooldownEndTime;
    
    Weapon(PlayerProjectile bullet, double cooldown, float spread, int bulletsPerShot, float bulletSize);
};
