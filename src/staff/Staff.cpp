#include "staff/Staff.hpp"

#include <algorithm>
#include <stdexcept>

namespace {
constexpr unsigned int kMaxFatigue = 100U;
constexpr unsigned int kNightHourStart = 22U;
constexpr unsigned int kNightHourEnd = 6U;

[[nodiscard]] bool is_night_hour(unsigned int hour_of_day) noexcept {
  const unsigned int h = hour_of_day % 24U;
  return h >= kNightHourStart || h < kNightHourEnd;
}

[[nodiscard]] unsigned int shift_duration_hours(const Shift& s) noexcept
{
    if (s.start_hour == 0U && s.end_hour == 0U) {
        return 0U;
    }
    if (s.start_hour < s.end_hour) {
        return s.end_hour - s.start_hour;
    }
    if (s.start_hour > s.end_hour) {
        return (24U - s.start_hour) + s.end_hour;
    }
    return 0U;
}
} // namespace

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

StaffRole Staff::get_role() const noexcept
{
    return role();
}

bool Staff::is_tired() const noexcept
{
    return fatigue >= kMaxFatigue;
}

unsigned int Staff::get_fatigue() const noexcept
{
    return fatigue;
}

bool Staff::process_fatigue(const Task& task, Weekday weekday, unsigned int hour_of_day) noexcept
{
    if (!can_handle(task, weekday, hour_of_day) || is_tired()) {
        return false;
    }
    return true;
}

unsigned int Staff::calculate_task_payment(const Task& task, unsigned int role_bonus) const noexcept
{
    return task.bonus + role_bonus;
}

unsigned int Staff::shift_hourly_rate(unsigned int hour_of_day,
                                      unsigned int weekday_index) const noexcept
{
    const Shift& shift = working_hours[weekday_index % 7U];
    if (shift.start_hour == 0U && shift.end_hour == 0U) {
        return is_night_hour(hour_of_day) ? salary + salary_night : salary;
    }
    if (shift.night_shift) {
        return salary + salary_night;
    }
    return salary;
}

unsigned int Staff::accrue_shift_pay(unsigned int elapsed_minutes,
                                     unsigned int hour_of_day,
                                     unsigned int weekday_index) noexcept
{
    if (!is_on_shift(hour_of_day, weekday_index)) {
        return 0U;
    }
    const unsigned int rate = shift_hourly_rate(hour_of_day, weekday_index);
    const unsigned int pay = (rate * elapsed_minutes) / 60U;
    if (pay > 0U) {
        to_be_paid += pay;
    }
    return pay;
}

void Staff::rest_tick(unsigned int elapsed_minutes) noexcept
{
    if (fatigue == 0U) {
        return;
    }
    const unsigned int delta = (fatigue_rate * elapsed_minutes) / (60U * 10U);
    fatigue = fatigue > delta ? fatigue - delta : 0U;
}

void Staff::increase_fatigue(unsigned int extra) noexcept
{
    fatigue = std::min(fatigue + (fatigue_rate * extra), kMaxFatigue);
}

void Staff::store_day_shift(Weekday day, Shift shift_type)
{
    if (shift_type.start_hour == shift_type.end_hour && shift_type.start_hour != 0U) {
        throw std::invalid_argument("Wrong shift type");
    }

    const unsigned int day_index = get_day(day);

    if (shift_type.start_hour == 0U && shift_type.end_hour == 0U) {
        shift_type.weekday = day;
        working_hours[day_index] = shift_type;
        return;
    }

    const unsigned int dur = shift_duration_hours(shift_type);
    if (dur > max_shift_duration_hours()) {
        throw std::invalid_argument("Shift too long for this role.");
    }

    if (forbids_night_shift_schedule()) {
        if (shift_type.end_hour <= shift_type.start_hour) {
            throw std::invalid_argument("This role cannot work overnight shifts.");
        }
        const unsigned int s = shift_type.start_hour;
        const unsigned int e = shift_type.end_hour;
        if (s < 6U || s >= 22U || e > 22U || e < 6U) {
            throw std::invalid_argument("This role cannot work night hours (allowed window 06:00-22:00).");
        }
    }

    check_schedule(day_index, shift_type.start_hour, shift_type.end_hour);

    if (shift_type.end_hour < shift_type.start_hour) {
        shift_type.night_shift = true;
    }
    shift_type.weekday = day;
    working_hours[day_index] = shift_type;
}

bool Staff::is_on_shift(unsigned hour_of_day, unsigned weekday) const noexcept
{
    const Shift& s = working_hours[weekday % 7U];
    if (s.start_hour == 0U && s.end_hour == 0U) {
        return true;
    }
    const unsigned int h = hour_of_day % 24U;
    if (s.start_hour < s.end_hour) {
        return h >= s.start_hour && h < s.end_hour;
    }
    if (s.start_hour > s.end_hour) {
        return h >= s.start_hour || h < s.end_hour;
    }
    return h == s.start_hour;
}

void Staff::check_schedule(unsigned int day_index, unsigned int start_hour, unsigned int end_hour) const
{
    if (day_index > 6U) {
        throw std::out_of_range("Invalid day index");
    }
    if (start_hour == end_hour && start_hour != 0U) {
        throw std::invalid_argument("Invalid shift duration");
    }

    const unsigned int yesterday = (day_index + 6U) % 7U;
    const unsigned int tomorrow = (day_index + 1U) % 7U;

    const Shift& prev = working_hours[yesterday];
    if (prev.start_hour != 0U || prev.end_hour != 0U) {
        int gap_prev = 0;
        if (prev.start_hour > prev.end_hour) {
            gap_prev = static_cast<int>(start_hour) - static_cast<int>(prev.end_hour);
        } else {
            gap_prev = 24 - static_cast<int>(prev.end_hour) + static_cast<int>(start_hour);
        }
        if (gap_prev < 10) {
            throw std::invalid_argument("Insufficient rest from yesterday (min 10h)!");
        }
    }

    const Shift& next = working_hours[tomorrow];
    if (next.start_hour != 0U || next.end_hour != 0U) {
        int gap_next = 0;
        if (start_hour > end_hour) {
            gap_next = static_cast<int>(next.start_hour) - static_cast<int>(end_hour);
        } else {
            gap_next = 24 - static_cast<int>(end_hour) + static_cast<int>(next.start_hour);
        }
        if (gap_next < 8) {
            throw std::invalid_argument("Insufficient rest before tomorrow (min 8h)!");
        }
    }
}

unsigned int Staff::get_day(Weekday day) const
{
    switch (day) {
        case Weekday::monday:
            return 0U;
        case Weekday::tuesday:
            return 1U;
        case Weekday::wednesday:
            return 2U;
        case Weekday::thursday:
            return 3U;
        case Weekday::friday:
            return 4U;
        case Weekday::saturday:
            return 5U;
        case Weekday::sunday:
            return 6U;
    }
    throw std::invalid_argument("Invalid weekday");
}

bool Staff::get_is_busy() const noexcept
{
    return is_busy;
}

const Task& Staff::get_current_task() const noexcept
{
    return current_task;
}

void Staff::process_tick(unsigned int elapsed_minutes, unsigned int hour_of_day,
                         unsigned int weekday_index)
{
    accrue_shift_pay(elapsed_minutes, hour_of_day, weekday_index);

    if (is_busy) {
        if (remaining_task_minutes <= elapsed_minutes) {
            is_busy = false;
            remaining_task_minutes = 0;
            increase_fatigue(current_task.difficulty);
            const unsigned int bonus = receive_bonus();
            const unsigned int payment = calculate_task_payment(current_task, bonus);
            to_be_paid += payment;
        } else {
            remaining_task_minutes -= elapsed_minutes;
        }
    } else if (!is_on_shift(hour_of_day, weekday_index)) {
        rest_tick(elapsed_minutes);
    }
}
