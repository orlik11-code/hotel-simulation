#pragma once

#include "booking/Reservation.h"
#include "events/EventGenerator.h"
#include "events/HotelEvent.h"
#include "hotel/Hotel.h"
#include "simulation/SimEventLog.hpp"
#include "meal/MealTypes.hpp"

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct SimConfig {
  unsigned int ticsPerDay = 24;
  unsigned int minutesPerTick = 60;
  unsigned int max_ticks = 0;
  /** True % chance per tick (1–100 roll); max 30 in config. */
  unsigned int event_probability = 5;
  /** Invalid until set from config (date_start=YYYY-MM-DD). */
  std::chrono::year_month_day date_start{};
  std::vector<MealServiceWindow> meal_windows;
  /** Szansa 1–100, że gość z food_included przyjdzie na dany posiłek w danym dniu. */
  unsigned int meal_attendance_percent = 80U;
};

enum class MealPhase { Cooking, Ready };

struct PendingMealTask {
  Reservation *reservation = nullptr;
  MealType meal_type{};
  MealPhase phase = MealPhase::Cooking;
  unsigned int worker_uid = 0U;
};

// raz na dzien
struct MealAttendancePlan {
  Reservation *reservation = nullptr;
  MealType meal_type{};
  std::chrono::sys_days day{};
  bool will_attend = false;
  unsigned int planned_arrival_hour = 0U;
};

// przyszedl
struct MealServedRecord {
  Reservation *reservation = nullptr;
  MealType meal_type{};
  std::chrono::sys_days day{};
};

// Gość nie przyszedł
struct MealNoShowRecord {
  Reservation *reservation = nullptr;
  MealType meal_type{};
  std::chrono::sys_days day{};
};


class Simulation {
private:
  Hotel hotel;
  SimConfig config;
  unsigned int current_tick;
  bool is_running;
  unsigned int time_with_fail = 0;
  std::unique_ptr<EventGenerator> event_generator;
  /** Aktywne zdarzenie do clear() po naprawie konserwatora. */
  std::unique_ptr<HotelEvent> active_event_;
  std::vector<Reservation *> pending_cancellations;

  /** Tymczasowy cout w log_event; docelowo EventLogger (Konrad). */
  bool live_console_output_ = false;

  // meals logic
  std::vector<MealAttendancePlan> meal_attendance_plans_;
  std::vector<PendingMealTask> pending_meals_;
  std::vector<MealServedRecord> meals_served_;
  std::vector<MealNoShowRecord> meal_no_shows_;

  std::optional<std::chrono::sys_days> last_daily_dirty_date;

  struct PendingReceptionTask {
    Reservation *reservation = nullptr;
    bool is_checkout = false;
    unsigned int worker_uid = 0U;
  };
  std::vector<PendingReceptionTask> pending_reception;

  SimEventLog event_log_;

  void log_event(const std::string &category, const std::string &message);

  [[nodiscard]] std::string format_virtual_time() const;

  void try_fix_failure();
  void process_reception_completions();
  void check_checkout(unsigned int hour, unsigned int day);
  void process_cancellations();
  [[nodiscard]] bool has_pending_reception(Reservation *reservation) const;
  void apply_guest_satisfaction_penalty(Reservation *reservation,
                                        int penalty_points);
  [[nodiscard]] std::chrono::sys_days
  get_actual_date(unsigned int days_elapsed) const noexcept;

  [[nodiscard]] bool should_run_daily_dirty(unsigned int hour_of_day,
                                          std::chrono::sys_days today) const;
  void process_daily_dirty_rooms(std::chrono::sys_days today);

  [[nodiscard]] std::optional<MealType> current_meal_slot(unsigned int hour) const;
  [[nodiscard]] const MealServiceWindow *
  find_meal_window(MealType meal_type) const noexcept;
  [[nodiscard]] bool has_meal_plan(Reservation *reservation, MealType meal_type,
                                   std::chrono::sys_days day) const;
  [[nodiscard]] bool already_served(Reservation *reservation, MealType meal_type,
                                    std::chrono::sys_days day) const;
  [[nodiscard]] bool has_no_show(Reservation *reservation, MealType meal_type,
                                 std::chrono::sys_days day) const;
  [[nodiscard]] bool has_pending_meal(Reservation *reservation,
                                      MealType meal_type) const;
  [[nodiscard]] const MealAttendancePlan *
  find_meal_plan(Reservation *reservation, MealType meal_type,
                 std::chrono::sys_days day) const;
  void ensure_meal_attendance_plans(MealType slot, std::chrono::sys_days today);
  void process_meal_completions();
  void dispatch_meals(MealType slot, unsigned int hour, unsigned int day);

public:
  Simulation(const SimConfig &sim_config);

  void set_config(const SimConfig &sim_config);

  /** Wlacza tymczasowy wypis zdarzen na cout (np. w main / HotelApp). */
  void set_live_console_output(bool enabled) noexcept;

  void run();
  void tick();
  [[nodiscard]] unsigned int minutes_of_hour() const noexcept;

  Hotel &get_hotel();

  void schedule_cancellation(Reservation *reservation);

  /** Bufor zdarzeń — bez EventLoggera; Konrad tylko odczytuje i renderuje. */
  [[nodiscard]] const SimEventLog &get_event_log() const noexcept;

  [[nodiscard]] unsigned int get_current_tick() const noexcept;
  [[nodiscard]] unsigned int days_elapsed() const noexcept;
  [[nodiscard]] std::chrono::sys_days actual_date() const noexcept;
  [[nodiscard]] unsigned int hour_of_day() const noexcept;
  [[nodiscard]] unsigned int minutes_from_midnight() const noexcept;
  [[nodiscard]] unsigned int weekday() const noexcept;
  [[nodiscard]] unsigned int minutes_per_tick() const noexcept;
};
