#include "Enemy.hpp"

Enemy::Enemy(int dmg, double cooldown, float maxSpeed, float acceleration, bool blocksPiercing)
: attackDamage{dmg}
, attackCooldown{cooldown}
, maxMovementSpeed{maxSpeed}
, acceleration{acceleration}
, cooldownEndTime{0}
, blocksPiercing{blocksPiercing} {}
