#include "PlayerProjectile.hpp"

PlayerProjectile::PlayerProjectile(int dmg, float speed, float lifespan, float despawnTime)
: damage{dmg}
, speed{speed}
, lifespan{lifespan}
, despawnTime{despawnTime} {}
