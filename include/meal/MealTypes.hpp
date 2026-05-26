#pragma once

#include <string>

enum class MealType { breakfast, dinner };


struct MealServiceWindow
{
    MealType type;
    unsigned int start_hour;
    unsigned int end_hour;
};

[[nodiscard]] MealType parse_meal_type(const std::string& value, unsigned int line_no);