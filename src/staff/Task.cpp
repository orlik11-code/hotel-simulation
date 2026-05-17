#include "staff/types.hpp"

#include <random>

unsigned int get_random_delta()
{
    static std::random_device randomize;
    static std::mt19937 gen(randomize());
    static std::uniform_int_distribution<unsigned int> dist(0, 60);
    return dist(gen);
}

Task::Task(TaskType t, unsigned int base_minutes)
    : type(t),
      duration_minutes(base_minutes + get_random_delta()),
      difficulty(1U),
      bonus(0U),
      night_shift(false)
{
}
