#include "Enemy.hpp"

Enemy::Enemy(int dmg, double cooldown, float maxSpeed, float acceleration)
: attackDamage{dmg}
, attackCooldown{cooldown}
, maxMovementSpeed{maxSpeed}
, acceleration{acceleration}
, cooldownEndTime{0} {}
