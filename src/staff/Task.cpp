#include "simulation/simulation_rng.hpp"
#include "staff/types.hpp"

namespace {

[[nodiscard]] unsigned int random_minutes(unsigned int min_minutes,
                                          unsigned int max_minutes) {
  return random_unsigned(min_minutes, max_minutes);
}

constexpr unsigned int kReceptionistTaskMinMinutes = 1U;
constexpr unsigned int kReceptionistTaskMaxMinutes = 10U;

constexpr unsigned int kHousekeeperTaskMinMinutes = 5U;
constexpr unsigned int kHousekeeperTaskMaxMinutes = 25U;

constexpr unsigned int kMaintanceWorkerMinMinutes = 5U;
constexpr unsigned int kMaintanceWorkerMaxMinutes = 30U;

constexpr unsigned int kCookMealMinMinutes = 10U;
constexpr unsigned int kCookMealMaxMinutes = 25U;

constexpr unsigned int kServeMealMinMinutes = 5U;
constexpr unsigned int kServeMealMaxMinutes = 15U;

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

Task Task::make_check_out()
{
    Task task;
    task.type = TaskType::CheckOut;
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


Task Task::make_fix_failure()
{
    Task task;
    task.type = TaskType::FixFailure;
    task.duration_minutes =
        random_minutes(kMaintanceWorkerMinMinutes, kMaintanceWorkerMaxMinutes);
    return task;
}

Task Task::make_cook_meal() {
  Task task;
  task.type = TaskType::CookMeal;
  task.duration_minutes =
      random_minutes(kCookMealMinMinutes, kCookMealMaxMinutes);
  return task;
}

Task Task::make_serve_meal() {
  Task task;
  task.type = TaskType::ServeMeal;
  task.duration_minutes =
      random_minutes(kServeMealMinMinutes, kServeMealMaxMinutes);
  return task;
}