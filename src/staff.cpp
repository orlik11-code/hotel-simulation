#include "../include/staff.hpp"
#include <algorithm>
#include <iostream>

namespace {
constexpr unsigned int kMaxFatigue = 100U;
}

Staff::Staff(unsigned int new_uid,
             const std::string& first_name,
             const std::string& last_name,
             unsigned int base_salary,
             unsigned int night_salary,
             unsigned int new_fatigue_rate)
    : uid(new_uid),
      to_be_paid(0U),
      salary(base_salary),
      salary_night(night_salary),
      name(first_name),
      surname(last_name),
      working_hours(),
      fatigue(0U),
      fatigue_rate(new_fatigue_rate)
{
    std::ranges::fill(working_hours, Shift{});
}

std::string Staff::get_first_name() const noexcept
{
    return name;
}

std::string Staff::get_last_name() const noexcept
{
    return surname;
}

unsigned int Staff::get_salary() const noexcept
{
    return salary;
}

unsigned int Staff::get_to_pay() const noexcept
{
    return to_be_paid;
}

unsigned int Staff::get_uid() const noexcept
{
    return uid;
}

bool Staff::is_tired() const noexcept
{
    return fatigue >= kMaxFatigue;
}

bool Staff::process_fatigue(const Task& task) noexcept
{
    if (!can_handle(task) || is_tired()) {
        return false;
    }
    increase_fatigue(task.difficulty);
    return true;
}

unsigned int Staff::calculate_task_payment(const Task& task, unsigned int role_bonus) const noexcept
{
    const unsigned int hourly_rate = task.night_shift ? salary + salary_night : salary;
    const unsigned int time_cost = hourly_rate * task.duration_hours;
    return time_cost + task.bonus + role_bonus;
}

void Staff::rest_tick() noexcept
{
    fatigue = fatigue > fatigue_rate ? fatigue - fatigue_rate : 0U;
}

void Staff::increase_fatigue(unsigned int extra) noexcept
{
    fatigue = std::min(fatigue + (fatigue_rate * extra), kMaxFatigue);
}

StaffRole Receptionist::role() const noexcept
{
    return StaffRole::Receptionist;
}

bool Receptionist::can_handle(const Task& task) const noexcept
{
    return task.type == TaskType::CheckIn;
}

void Receptionist::do_work(const Task& task)
{
    if (!process_fatigue(task)) {
        std::cout << "Receptionist " << name << " cannot process this task right now.\n";
        return;
    }
    const unsigned int role_bonus = task.difficulty;
    to_be_paid += calculate_task_payment(task, role_bonus);
    std::cout << "Receptionist " << name << " checked in a guest.\n";
}

StaffRole Housekeeper::role() const noexcept
{
    return StaffRole::Housekeeper;
}

bool Housekeeper::can_handle(const Task& task) const noexcept
{
    return task.type == TaskType::CleanRoom && !task.night_shift;
}

void Housekeeper::do_work(const Task& task)
{
    if (!process_fatigue(task)) {
        std::cout << "Housekeeper " << name << " cannot process this task right now.\n";
        return;
    }
    const unsigned int role_bonus = 2U * task.difficulty;
    to_be_paid += calculate_task_payment(task, role_bonus);
    std::cout << "Housekeeper " << name << " cleaned a room.\n";
}

StaffRole Chef::role() const noexcept
{
    return StaffRole::Chef;
}

bool Chef::can_handle(const Task& task) const noexcept
{
    return task.type == TaskType::CookMeal;
}

void Chef::do_work(const Task& task)
{
    if (!process_fatigue(task)) {
        std::cout << "Chef " << name << " cannot process this task right now.\n";
        return;
    }
    const unsigned int role_bonus = 3U * task.difficulty;
    to_be_paid += calculate_task_payment(task, role_bonus);
    std::cout << "Chef " << name << " prepared a meal.\n";
}

StaffRole Waiter::role() const noexcept
{
    return StaffRole::Waiter;
}

bool Waiter::can_handle(const Task& task) const noexcept
{
    return task.type == TaskType::ServeMeal;
}

void Waiter::do_work(const Task& task)
{
    if (!process_fatigue(task)) {
        std::cout << "Waiter " << name << " cannot process this task right now.\n";
        return;
    }
    const unsigned int role_bonus = task.difficulty;
    to_be_paid += calculate_task_payment(task, role_bonus);
    std::cout << "Waiter " << name << " served an order.\n";
}

StaffRole MaintenanceWorker::role() const noexcept
{
    return StaffRole::MaintenanceWorker;
}

bool MaintenanceWorker::can_handle(const Task& task) const noexcept
{
    return task.type == TaskType::FixFailure && !task.night_shift;
}

void MaintenanceWorker::do_work(const Task& task)
{
    if (!process_fatigue(task)) {
        std::cout << "Maintenance worker " << name << " cannot process this task right now.\n";
        return;
    }
    const unsigned int role_bonus = 4U * task.difficulty;
    to_be_paid += calculate_task_payment(task, role_bonus);
    std::cout << "Maintenance worker " << name << " fixed a failure.\n";
}
