#pragma once

#include <string>

enum class Weekday {
    monday,
    tuesday,
    wednesday,
    thursday,
    friday,
    saturday,
    sunday
};

enum class StaffRole {
    Receptionist,
    Housekeeper,
    Chef,
    Waiter,
    MaintenanceWorker
};

enum class TaskType {
    CheckIn,
    CleanRoom,
    CookMeal,
    ServeMeal,
    FixFailure
};

struct Shift {
    Weekday weekday;
    unsigned int start_hour = 0;
    unsigned int end_hour = 0;
    bool night_shift = false;
};

[[nodiscard]] unsigned int get_random_delta();

struct Task {
    TaskType type = TaskType::CheckIn;
    unsigned int duration_minutes = 0;
    unsigned int difficulty = 1;
    unsigned int bonus = 0;
    bool night_shift = false;

    Task() = default;

    /** Domyślnie 1 h pracy (60 min), bez losowości — wygodne w testach `Task{Typ}`. */
    explicit Task(TaskType t)
        : type(t),
          duration_minutes(60U)
    {
    }

    /** Jak stary init list: (typ, duration_w_godzinach, difficulty, bonus, night_shift). */
    Task(TaskType t, unsigned int duration_hours, unsigned int diff, unsigned int task_bonus, bool night)
        : type(t),
          duration_minutes(duration_hours * 60U),
          difficulty(diff),
          bonus(task_bonus),
          night_shift(night)
    {
    }

    /** Bazowe minuty + losowy dodatek — implementacja w src/staff/Task.cpp. */
    Task(TaskType t, unsigned int base_minutes);
};
