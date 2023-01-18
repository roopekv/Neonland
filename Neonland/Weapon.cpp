#include "Weapon.hpp"

Weapon::Weapon(PlayerProjectile projectile, double cooldown, float spread, int projectilesPerShot, float projectileSize, Mesh projectileMesh)
: projectile{projectile}
, cooldown{cooldown}
, spread{spread}
, projectilesPerShot{projectilesPerShot}
, projectileSize{projectileSize}
, projectileMesh{projectileMesh}
, cooldownEndTime{0} {}
