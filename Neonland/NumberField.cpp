#include "NumberField.hpp"

#include <string>
#include <cctype>

unsigned int NumberField::GetValue() const {
    return value;
}

void NumberField::SetValue(unsigned int value) {
    this->value = value;
    auto numString = std::to_string(value);
    
    valueNums.clear();
    
    for (size_t i = 0; i < valueUIEntities.size(); i++) {
        if (i < numString.length() && isdigit(numString[i])) {
            valueNums.insert(valueNums.begin(), numString[i] - '0');
        }
        else {
            valueNums.push_back(10);
        }
    }
}

void NumberField::SetMaxValue(unsigned int maxValue) {
    this->maxValue = maxValue;
    auto numString = std::to_string(maxValue);
    
    maxValueNums.clear();
    
    for (size_t i = 0; i < maxValueUIEntities.size(); i++) {
        if (i < numString.length() && isdigit(numString[i])) {
            maxValueNums.push_back(numString[i] - '0');
        }
        else {
            maxValueNums.push_back(static_cast<TextureType>(10));
        }
    }
}
unsigned int NumberField::GetMaxValue() const {
    return maxValue;
}

