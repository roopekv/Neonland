#include "NumberField.hpp"

#include <string>
#include <cctype>

int32_t NumberField::GetValue() const {
    return value;
}

void NumberField::SetValue(int32_t value) {
    this->value = value;
    auto numString = std::to_string(value);
    
    valueNums.clear();
    
    for (char c : numString) {
        if (isdigit(c)) {
            valueNums.push_back(c - '0');
        }
    }
}

