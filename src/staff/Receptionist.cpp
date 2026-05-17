#include "staff/Receptionist.hpp"

#include <iostream>

unsigned int Receptionist::max_shift_duration_hours() const noexcept
{
    return 12U;
}

StaffRole Receptionist::role() const noexcept
{
    return StaffRole::Receptionist;
}

bool Receptionist::can_handle(const Task& task, Weekday weekday, unsigned int hour_of_day) const noexcept
{
    const unsigned int idx = get_day(weekday);
    if (!is_on_shift(hour_of_day, idx)) {
        return false;
    }
    return task.type == TaskType::CheckIn;
}

void Receptionist::do_work(const Task& task, Weekday weekday, unsigned int hour_of_day)
{
    if (!process_fatigue(task, weekday, hour_of_day)) {
        std::cout << "Receptionist " << name << " cannot process this task right now.\n";
        return;
    }
    is_busy = true;
    current_task = task;
    remaining_task_minutes = task.duration_minutes;
    std::cout << "Receptionist " << name << " checked in a guest.\n";
}

void Receptionist::determine_schedule(Weekday day, Shift shift_type)
{
    store_day_shift(day, shift_type);
}

unsigned int Receptionist::receive_bonus() const noexcept
{
    return current_task.difficulty / 4U;
}
