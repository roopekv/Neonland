#include "Enemy.hpp"

Enemy::Enemy(int dmg, double cooldown)
: attackDamage{dmg}
, attackCooldown{cooldown}
, cooldownEndTime{0} {}
