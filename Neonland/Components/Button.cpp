#include "Button.hpp"

Button::Button(std::function<void()> onClick, bool enabled)
: onClick(onClick)
, enabled{enabled}
, highlighted{false} {}
