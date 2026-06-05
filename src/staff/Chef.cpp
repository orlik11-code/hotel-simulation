#include "staff/Chef.hpp"

unsigned int Chef::max_shift_duration_hours() const noexcept
{
    return 6U;
}

bool Chef::forbids_night_shift_schedule() const noexcept
{
    return true;
}

StaffRole Chef::role() const noexcept
{
    return StaffRole::Chef;
}

bool Chef::can_handle(const Task& task, Weekday weekday, unsigned int hour_of_day) const noexcept
{
    const unsigned int idx = get_day(weekday);
    if (!is_on_shift(hour_of_day, idx)) {
        return false;
    }
    return task.type == TaskType::CookMeal;
}

void Chef::do_work(const Task& task, Weekday weekday, unsigned int hour_of_day)
{
    if (!process_fatigue(task, weekday, hour_of_day)) {
        return;
    }
    is_busy = true;
    remaining_task_minutes = task.duration_minutes;
    current_task = task;
}

void Chef::determine_schedule(Weekday day, Shift shift_type)
{
    store_day_shift(day, shift_type);
}

unsigned int Chef::receive_bonus() const noexcept
{
    return (current_task.difficulty * 2U) / 3U;
}
