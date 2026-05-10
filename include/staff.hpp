#pragma once
#include <array>
#include <string>


enum class Weekday
{
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

    /** Bazowe minuty + losowy dodatek — implementacja w staff.cpp (get_random_delta). */
    Task(TaskType t, unsigned int base_minutes);
};


// Abstrakcyjna klasa bazowa personelu: wspólne dane i interfejs pracy.
class Staff {
    protected:
        // uid
        unsigned int uid;
        // to be pay
        unsigned int to_be_paid;
        // salary
        unsigned int salary;
        unsigned int salary_night;

        //staff_data
        std::string name;
        std::string surname;

        //working_hours
        std::array<Shift, 7> working_hours;
        // working
        bool is_busy = false;
        /** Pozostały czas bieżącego zadania w minutach (duration_hours * 60 przy starcie). */
        unsigned int remaining_task_minutes = 0;
        Task current_task;


        //fatigue
        unsigned int fatigue;
        unsigned int fatigue_rate;

        bool process_fatigue(const Task& task, Weekday weekday, unsigned int hour_of_day) noexcept;
        [[nodiscard]] unsigned int calculate_task_payment(const Task& task, unsigned int role_bonus) const noexcept;

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

        // MAIN METHOD
        /** elapsed_minutes — ile minut modelu mija w tym ticku (z SimConfig::minutesPerTick). */
        void process_tick(unsigned int elapsed_minutes);

        //getters basic
        [[nodiscard]] unsigned int get_uid() const noexcept;
        [[nodiscard]] std::string get_first_name() const noexcept;
        [[nodiscard]] std::string get_last_name() const noexcept;
        [[nodiscard]] StaffRole get_role() const noexcept;

        // money
        [[nodiscard]] unsigned int get_salary() const noexcept;
        [[nodiscard]] unsigned int get_to_pay() const noexcept;

        [[nodiscard]] bool is_tired() const noexcept;
        [[nodiscard]] unsigned int get_fatigue() const noexcept;
        void rest_tick(unsigned int elapsed_minutes) noexcept;
        void increase_fatigue(unsigned int extra = 1U) noexcept;

        [[nodiscard]] virtual StaffRole role() const noexcept = 0;
        [[nodiscard]] virtual bool can_handle(const Task& task, Weekday weekday, unsigned int hour_of_day) const noexcept = 0;
        virtual void do_work(const Task& task, Weekday weekday, unsigned int hour_of_day) = 0;

        /** weekday: 0 = poniedziałek … 6 = niedziela (dowolna ustalona konwencja). */
        [[nodiscard]] bool is_on_shift(unsigned hour_of_day, unsigned weekday) const noexcept;
        void check_schedule(unsigned int day_index, unsigned int start_hour, unsigned int end_hour) const;
        virtual void determine_schedule(Weekday day, Shift shift_type) = 0;

        [[nodiscard]] unsigned int get_day(Weekday day) const;

        // being busy
        [[nodiscard]] bool get_is_busy() const noexcept;


        virtual unsigned int receive_bonus() const noexcept = 0;
};

// Recepcjonista: obsługuje zadania związane z meldowaniem gości.
class Receptionist final : public Staff {
    protected:
        [[nodiscard]] unsigned int max_shift_duration_hours() const noexcept override;

    public:
        using Staff::Staff;
        [[nodiscard]] StaffRole role() const noexcept override;
        [[nodiscard]] bool can_handle(const Task& task, Weekday weekday, unsigned int hour_of_day) const noexcept override;
        void do_work(const Task& task, Weekday weekday, unsigned int hour_of_day) override;
        void determine_schedule(Weekday day, Shift shift_type) override;

        unsigned int receive_bonus() const noexcept override;
};

// Pokojówka: realizuje zadania sprzątania pokoi.
class Housekeeper final : public Staff {
    protected:
        [[nodiscard]] unsigned int max_shift_duration_hours() const noexcept override;
        [[nodiscard]] bool forbids_night_shift_schedule() const noexcept override;

    public:
        using Staff::Staff;
        [[nodiscard]] StaffRole role() const noexcept override;
        [[nodiscard]] bool can_handle(const Task& task, Weekday weekday, unsigned int hour_of_day) const noexcept override;
        void do_work(const Task& task, Weekday weekday, unsigned int hour_of_day) override;
        void determine_schedule(Weekday day, Shift shift_type) override;
        unsigned int receive_bonus() const noexcept override;
};

// Kucharz: przygotowuje posiłki dla gości hotelu.
class Chef final : public Staff {
    protected:
        [[nodiscard]] unsigned int max_shift_duration_hours() const noexcept override;
        [[nodiscard]] bool forbids_night_shift_schedule() const noexcept override;

    public:
        using Staff::Staff;
        [[nodiscard]] StaffRole role() const noexcept override;
        [[nodiscard]] bool can_handle(const Task& task, Weekday weekday, unsigned int hour_of_day) const noexcept override;
        void do_work(const Task& task, Weekday weekday, unsigned int hour_of_day) override;
        void determine_schedule(Weekday day, Shift shift_type) override;
        unsigned int receive_bonus() const noexcept override;
};

// Kelner: dostarcza i serwuje gotowe zamówienia.
class Waiter final : public Staff {
    protected:
        [[nodiscard]] unsigned int max_shift_duration_hours() const noexcept override;

    public:
        using Staff::Staff;
        [[nodiscard]] StaffRole role() const noexcept override;
        [[nodiscard]] bool can_handle(const Task& task, Weekday weekday, unsigned int hour_of_day) const noexcept override;
        void do_work(const Task& task, Weekday weekday, unsigned int hour_of_day) override;
        void determine_schedule(Weekday day, Shift shift_type) override;
        unsigned int receive_bonus() const noexcept override;
};

// Konserwator: usuwa awarie i usterki techniczne.
class MaintenanceWorker final : public Staff {
    protected:
        [[nodiscard]] unsigned int max_shift_duration_hours() const noexcept override;

    public:
        using Staff::Staff;
        [[nodiscard]] StaffRole role() const noexcept override;
        [[nodiscard]] bool can_handle(const Task& task, Weekday weekday, unsigned int hour_of_day) const noexcept override;
        void do_work(const Task& task, Weekday weekday, unsigned int hour_of_day) override;
        void determine_schedule(Weekday day, Shift shift_type) override;
        unsigned int receive_bonus() const noexcept override;
};

