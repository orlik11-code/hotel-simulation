#include "meal/MealTypes.hpp"

#include <stdexcept>

MealType parse_meal_type(const std::string &value, unsigned int line_no) {
  if (value == "breakfast") {
    return MealType::breakfast;
  }
  if (value == "dinner") {
    return MealType::dinner;
  }
  throw std::invalid_argument("Line " + std::to_string(line_no) +
                              ": unknown meal type '" + value + "'");
}
