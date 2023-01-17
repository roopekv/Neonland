#include "PlayerProjectile.hpp"

PlayerProjectile::PlayerProjectile(int dmg, float lifespan, float speed, bool destructsOnCol)
: damage{dmg}
, lifespan{lifespan}
, speed{speed}
, destructsOnCollision{destructsOnCol}
, despawnTime{0} {}
