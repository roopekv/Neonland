#include "Button.hpp"

Button::Button(std::function<void()> onClick)
: onClick(onClick)
, highlighted{false}
, disabled{false} {}
