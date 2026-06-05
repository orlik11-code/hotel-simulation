#include "staff/Waiter.hpp"

unsigned int Waiter::max_shift_duration_hours() const noexcept
{
    return 12U;
}

StaffRole Waiter::role() const noexcept
{
    return StaffRole::Waiter;
}

bool Waiter::can_handle(const Task& task, Weekday weekday, unsigned int hour_of_day) const noexcept
{
    const unsigned int idx = get_day(weekday);
    if (!is_on_shift(hour_of_day, idx)) {
        return false;
    }
    return task.type == TaskType::ServeMeal;
}

void Waiter::do_work(const Task& task, Weekday weekday, unsigned int hour_of_day)
{
    if (!process_fatigue(task, weekday, hour_of_day)) {
        return;
    }
    is_busy = true;
    remaining_task_minutes = task.duration_minutes;
    current_task = task;
}

void Waiter::determine_schedule(Weekday day, Shift shift_type)
{
    store_day_shift(day, shift_type);
}

unsigned int Waiter::receive_bonus() const noexcept
{
    return (current_task.difficulty / 5U) * 8U;
}
