#include "NumberField.hpp"

#include <string>
#include <cctype>

unsigned int NumberField::GetValue() const {
    return value;
}

void NumberField::SetValue(unsigned int value) {
    this->value = value;
    auto numString = std::to_string(value);
    
    for (size_t i = 0; i < numberEntities.size(); i++) {
        if (i < numString.length() && isdigit(numString[i])) {
            numbers[i] = numString[i] - '0';
        }
        else {
            numbers[i] = 10;
        }
    }
}

