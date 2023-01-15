#include "Weapon.hpp"

Weapon::Weapon(PlayerProjectile bullet, double cooldown, float spread, int bulletsPerShot, float bulletSize)
: bullet{bullet}
, cooldown{cooldown}
, spread{spread}
, bulletsPerShot{bulletsPerShot}
, bulletSize(bulletSize)
, cooldownEndTime{0} {}
