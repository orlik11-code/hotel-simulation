#include "staff/types.hpp"

#include <random>

namespace {

std::mt19937& task_rng()
{
    static std::random_device randomize;
    static std::mt19937 gen(randomize());
    return gen;
}

[[nodiscard]] unsigned int random_minutes(unsigned int min_minutes,
                                            unsigned int max_minutes)
{
    std::uniform_int_distribution<unsigned int> dist(min_minutes, max_minutes);
    return dist(task_rng());
}

constexpr unsigned int kReceptionistTaskMinMinutes = 1U;
constexpr unsigned int kReceptionistTaskMaxMinutes = 10U;

constexpr unsigned int kHousekeeperTaskMinMinutes = 5U;
constexpr unsigned int kHousekeeperTaskMaxMinutes = 25U;

constexpr unsigned int kRandomDeltaMaxMinutes = 20U;

} // namespace

unsigned int get_random_delta()
{
    return random_minutes(0U, kRandomDeltaMaxMinutes);
}

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
    task.duration_minutes =
        random_minutes(kReceptionistTaskMinMinutes, kReceptionistTaskMaxMinutes);
    return task;
}

Task Task::make_clean_room()
{
    Task task;
    task.type = TaskType::CleanRoom;
    task.duration_minutes =
        random_minutes(kHousekeeperTaskMinMinutes, kHousekeeperTaskMaxMinutes);
    return task;
}
