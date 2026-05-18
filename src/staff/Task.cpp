#include "staff/types.hpp"

#include <random>

namespace {

constexpr unsigned int kReceptionistTaskMinMinutes = 1U;
constexpr unsigned int kReceptionistTaskMaxMinutes = 10U;

[[nodiscard]] unsigned int random_receptionist_task_minutes()
{
    static std::random_device randomize;
    static std::mt19937 gen(randomize());
    static std::uniform_int_distribution<unsigned int> dist(
        kReceptionistTaskMinMinutes, kReceptionistTaskMaxMinutes);
    return dist(gen);
}

} // namespace

unsigned int get_random_delta()
{
    static std::random_device randomize;
    static std::mt19937 gen(randomize());
    static std::uniform_int_distribution<unsigned int> dist(0, 60);
    return dist(gen);
}

// as standard
Task::Task(TaskType t, unsigned int base_minutes)
    : type(t),
      duration_minutes(base_minutes + get_random_delta()),
      difficulty(1U),
      bonus(0U),
      night_shift(false)
{
}

Task Task::make_check_in()
{
    Task task;
    task.type = TaskType::CheckIn;
    task.duration_minutes = random_receptionist_task_minutes();
    return task;
}
