#pragma once

#include "events/EventGenerator.h"
#include "hotel/Hotel.h"

#include <chrono>
#include <memory>
#include <optional>
#include <vector>

struct SimConfig {
  unsigned int ticsPerDay = 24;
  unsigned int minutesPerTick = 60;
  unsigned int max_ticks = 0;
  /** True % chance per tick (1–100 roll); max 30 in config. */
  unsigned int event_probability = 5;
  /** Invalid until set from config (date_start=YYYY-MM-DD). */
  std::chrono::year_month_day date_start{};
};

class Simulation {
private:
  Hotel hotel;
  SimConfig config;
  unsigned int current_tick;
  bool is_running;
  unsigned int time_with_fail = 0;
  std::unique_ptr<EventGenerator> event_generator;
  std::vector<Reservation *> pending_cancellations;

  /** Data ostatniego porannego batcha brudnych pokoi (operational day start). */
  std::optional<std::chrono::sys_days> last_daily_dirty_date;

  struct PendingReceptionTask {
    Reservation *reservation = nullptr;
    bool is_checkout = false;
    unsigned int worker_uid = 0U;
  };
  std::vector<PendingReceptionTask> pending_reception;

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

public:
  Simulation(const SimConfig &sim_config);

  void set_config(const SimConfig &sim_config);

  void run();
  void tick();

  Hotel &get_hotel();

  void schedule_cancellation(Reservation *reservation);

  [[nodiscard]] unsigned int get_current_tick() const noexcept;
  [[nodiscard]] unsigned int days_elapsed() const noexcept;
  [[nodiscard]] std::chrono::sys_days actual_date() const noexcept;
  [[nodiscard]] unsigned int hour_of_day() const noexcept;
  [[nodiscard]] unsigned int weekday() const noexcept;
  [[nodiscard]] unsigned int minutes_per_tick() const noexcept;
};
