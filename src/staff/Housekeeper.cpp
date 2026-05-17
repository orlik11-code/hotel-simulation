#include "staff/Housekeeper.hpp"

#include <iostream>

unsigned int Housekeeper::max_shift_duration_hours() const noexcept
{
    return 8U;
}

bool Housekeeper::forbids_night_shift_schedule() const noexcept
{
    return true;
}

StaffRole Housekeeper::role() const noexcept
{
    return StaffRole::Housekeeper;
}

bool Housekeeper::can_handle(const Task& task, Weekday weekday, unsigned int hour_of_day) const noexcept
{
    const unsigned int idx = get_day(weekday);
    if (!is_on_shift(hour_of_day, idx)) {
        return false;
    }
    return task.type == TaskType::CleanRoom && !task.night_shift;
}

void Housekeeper::do_work(const Task& task, Weekday weekday, unsigned int hour_of_day)
{
    if (!process_fatigue(task, weekday, hour_of_day)) {
        std::cout << "Housekeeper " << name << " cannot process this task right now.\n";
        return;
    }
    is_busy = true;
    remaining_task_minutes = task.duration_minutes;
    current_task = task;
    std::cout << "Housekeeper " << name << " cleaned a room.\n";
}

void Housekeeper::determine_schedule(Weekday day, Shift shift_type)
{
    store_day_shift(day, shift_type);
}

unsigned int Housekeeper::receive_bonus() const noexcept
{
    return (current_task.difficulty / 3U) * 4U;
}
