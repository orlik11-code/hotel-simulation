#pragma once

#include "events/EventGenerator.h"
#include "events/HotelEvent.h"
#include "hotel/Hotel.h"
#include "simulation/HotelReport.h"
#include "simulation/SimConfig.hpp"
#include "simulation/SimEventLog.hpp"
#include "simulation/SimulationTypes.hpp"

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Simulation {
public:
  explicit Simulation(const SimConfig &sim_config);

  void set_config(const SimConfig &sim_config);
  void set_live_console_output(bool enabled) noexcept;

  void run();
  void tick();

  Hotel &get_hotel();
  void schedule_cancellation(Reservation *reservation);

  [[nodiscard]] const SimEventLog &get_event_log() const noexcept;
  [[nodiscard]] const HotelReport &get_hotel_report() const noexcept;
  [[nodiscard]] unsigned int get_current_tick() const noexcept;
  [[nodiscard]] unsigned int days_elapsed() const noexcept;
  [[nodiscard]] std::chrono::sys_days actual_date() const noexcept;
  [[nodiscard]] unsigned int hour_of_day() const noexcept;
  [[nodiscard]] unsigned int minutes_from_midnight() const noexcept;
  [[nodiscard]] unsigned int minutes_of_hour() const noexcept;
  [[nodiscard]] unsigned int weekday() const noexcept;
  [[nodiscard]] unsigned int minutes_per_tick() const noexcept;
  [[nodiscard]] std::string format_virtual_time_for_tick(unsigned int tick) const;

private:
  // --- State ---
  Hotel hotel_;
  SimConfig config_;
  unsigned int current_tick_ = 0U;
  bool is_running_ = true;
  unsigned int time_with_fail_ = 0U;
  std::unique_ptr<EventGenerator> event_generator_;
  std::unique_ptr<HotelEvent> active_event_;
  std::vector<Reservation *> pending_cancellations_;
  bool live_console_output_ = false;

  std::vector<MealAttendancePlan> meal_attendance_plans_;
  std::vector<PendingMealTask> pending_meals_;
  std::vector<MealServedRecord> meals_served_;
  std::vector<MealNoShowRecord> meal_no_shows_;
  std::optional<std::chrono::sys_days> last_daily_dirty_date_;
  std::vector<PendingReceptionTask> pending_reception_;
  SimEventLog event_log_;
  HotelReport hotel_report_;

  // --- Logging and time ---
  void log_event(const std::string &category, const std::string &message);
  [[nodiscard]] bool should_print_to_console(const std::string &category) const;
  [[nodiscard]] std::string format_virtual_time() const;
  [[nodiscard]] std::chrono::sys_days
  date_from_day_offset(unsigned int days_elapsed) const noexcept;

  // --- Tick phases ---
  void tick_advance_hotel_clock();
  void tick_resolve_maintenance();
  void tick_process_reception_queue();
  void tick_process_failures_and_random_events();
  void tick_evaluate_hotel_cancellations(unsigned int hour, unsigned int minute);
  void tick_process_pending_cancellations();
  void tick_mark_daily_dirty_rooms();
  void tick_schedule_reception_tasks(unsigned int hour, unsigned int minute, unsigned int weekday);
  void tick_process_meals(unsigned int hour, unsigned int weekday);
  void tick_dispatch_housekeeping(unsigned int hour, unsigned int weekday);
  void tick_log_time_step(unsigned int hour, unsigned int weekday);

  // --- Failures ---
  void try_fix_failure();

  // --- Reception ---
  void complete_pending_reception_tasks();
  void process_guest_checkout_requests(std::chrono::sys_days today,
                                       unsigned int hour, unsigned int minute);
  void schedule_check_in_and_check_out(unsigned int hour, unsigned int minute, unsigned int weekday);
  [[nodiscard]] bool has_pending_reception(Reservation *reservation) const;
  void apply_guest_satisfaction_penalty(Reservation *reservation,
                                        int penalty_points);

  // --- Housekeeping ---
  [[nodiscard]] bool should_run_daily_dirty(unsigned int hour_of_day,
                                          std::chrono::sys_days today) const;
  void process_daily_dirty_rooms(std::chrono::sys_days today);

  // --- Meals ---
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
  void complete_pending_meal_tasks();
  void dispatch_meals(MealType slot, unsigned int hour, unsigned int weekday);

    // --- Report ---
  HotelReport build_hotel_report() const;
  void refresh_hotel_report();
  void publish_hotel_report() const;
  void export_log_file() const;

  void process_cancellations();
};
