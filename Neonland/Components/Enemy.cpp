#include "Enemy.hpp"

Enemy::Enemy(int dmg, double cooldown, float moveSpeed)
: attackDamage{dmg}
, attackCooldown{cooldown}
, movementSpeed{moveSpeed}
, cooldownEndTime{0} {}
