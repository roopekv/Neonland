#include "HP.hpp"

#include <algorithm>

HP::HP(int max) : maxHP{max}, hp{max} {}

void HP::Set(int hp) {
    this->hp = std::clamp(hp, 0, maxHP);
}

int HP::Get() const {
    return hp;
}

int HP::Max() const {
    return maxHP;
}

void HP::Decrease(int dmg) {
    Set(hp - dmg);
}

void HP::Increase(int heal) {
    Set(hp + heal);
}
