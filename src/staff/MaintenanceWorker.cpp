#include "staff/MaintenanceWorker.hpp"

namespace {
constexpr unsigned int kEmergencyCallPayout = 500U;
} // namespace

unsigned int MaintenanceWorker::max_shift_duration_hours() const noexcept
{
    return 8U;
}

StaffRole MaintenanceWorker::role() const noexcept
{
    return StaffRole::MaintenanceWorker;
}

bool MaintenanceWorker::can_handle(const Task& task, Weekday weekday, unsigned int hour_of_day) const noexcept
{
    const unsigned int idx = get_day(weekday);
    if (!is_on_shift(hour_of_day, idx)) {
        return false;
    }
    return task.type == TaskType::FixFailure && !task.night_shift;
}

void MaintenanceWorker::do_work(const Task& task, Weekday weekday, unsigned int hour_of_day)
{
    if (!process_fatigue(task, weekday, hour_of_day)) {
        return;
    }
    is_busy = true;
    remaining_task_minutes = task.duration_minutes;
    current_task = task;
}

void MaintenanceWorker::determine_schedule(Weekday day, Shift shift_type)
{
    store_day_shift(day, shift_type);
}

unsigned int MaintenanceWorker::receive_bonus() const noexcept
{
    return current_task.difficulty * 3U;
}

unsigned int MaintenanceWorker::calculate_task_payment(const Task& task,
                                                       unsigned int role_bonus) const noexcept
{
    (void)role_bonus;
    if (task.type == TaskType::FixFailure) {
        return kEmergencyCallPayout + task.bonus;
    }
    return Staff::calculate_task_payment(task, role_bonus);
}
