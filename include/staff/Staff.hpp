#pragma once

#include "staff/types.hpp"

#include <array>
#include <string>

/** Abstract staff base class: shared state and work interface. */
class Staff {
protected:
    unsigned int uid;
    unsigned int to_be_paid;
    unsigned int salary;
    unsigned int salary_night;

    std::string name;
    std::string surname;

    std::array<Shift, 7> working_hours;
    bool is_busy = false;
    /** Remaining time of the current task in minutes (duration_hours * 60 at start). */
    unsigned int remaining_task_minutes = 0;
    Task current_task;

    unsigned int fatigue;
    unsigned int fatigue_rate;

    bool process_fatigue(const Task& task, Weekday weekday, unsigned int hour_of_day) noexcept;
    [[nodiscard]] virtual unsigned int calculate_task_payment(const Task& task, unsigned int role_bonus) const noexcept;
    [[nodiscard]] unsigned int shift_hourly_rate(unsigned int hour_of_day,
                                               unsigned int weekday_index) const noexcept;
    [[nodiscard]] unsigned int accrue_shift_pay(unsigned int elapsed_minutes,
                                                unsigned int hour_of_day,
                                                unsigned int weekday_index) noexcept;

    void store_day_shift(Weekday day, Shift shift_type);

    [[nodiscard]] virtual unsigned int max_shift_duration_hours() const noexcept = 0;
    [[nodiscard]] virtual bool forbids_night_shift_schedule() const noexcept { return false; }

public:
    Staff(unsigned int new_uid, const std::string& first_name,
          const std::string& last_name,
          unsigned int base_salary,
          unsigned int night_salary,
          unsigned int fatigue_rate);
    virtual ~Staff() = default;

    /** elapsed_minutes — model minutes elapsed this tick (from SimConfig::minutesPerTick). */
    void process_tick(unsigned int elapsed_minutes, unsigned int hour_of_day,
                      unsigned int weekday_index);

    [[nodiscard]] unsigned int get_uid() const noexcept;
    [[nodiscard]] std::string get_first_name() const noexcept;
    [[nodiscard]] std::string get_last_name() const noexcept;
    [[nodiscard]] StaffRole get_role() const noexcept;

    [[nodiscard]] unsigned int get_salary() const noexcept;
    [[nodiscard]] unsigned int get_to_pay() const noexcept;

    [[nodiscard]] bool is_tired() const noexcept;
    [[nodiscard]] unsigned int get_fatigue() const noexcept;
    void rest_tick(unsigned int elapsed_minutes) noexcept;
    void increase_fatigue(unsigned int extra = 1U) noexcept;

    [[nodiscard]] virtual StaffRole role() const noexcept = 0;
    [[nodiscard]] virtual bool can_handle(const Task& task, Weekday weekday, unsigned int hour_of_day) const noexcept = 0;
    virtual void do_work(const Task& task, Weekday weekday, unsigned int hour_of_day) = 0;

    /** weekday: 0 = Monday … 6 = Sunday (fixed convention). */
    [[nodiscard]] bool is_on_shift(unsigned hour_of_day, unsigned weekday) const noexcept;
    void check_schedule(unsigned int day_index, unsigned int start_hour, unsigned int end_hour) const;
    virtual void determine_schedule(Weekday day, Shift shift_type) = 0;

    [[nodiscard]] unsigned int get_day(Weekday day) const;

    [[nodiscard]] bool get_is_busy() const noexcept;
    [[nodiscard]] const Task& get_current_task() const noexcept;

    virtual unsigned int receive_bonus() const noexcept = 0;
};
