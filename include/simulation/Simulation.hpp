#pragma once

#include "events/EventGenerator.h"
#include "hotel/Hotel.h"

#include <chrono>
#include <memory>

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

  void try_fix_failure();
  /** Recepcjoniści obsługują meldowania/wymeldowania w godzinach pracy hotelu.
   *  Pełne ciało jest na razie zakomentowane — czeka na Hotel::get_reservations()
   *  oraz Reservation::get_checkin()/get_checkout(). */
  void check_checkout(unsigned int hour, unsigned int day);
  [[nodiscard]] std::chrono::sys_days get_actual_date(unsigned int days_elapsed) const noexcept;

public:
  Simulation(const SimConfig &sim_config);

  void set_config(const SimConfig &sim_config);

  void run();
  void tick();

  Hotel &get_hotel();

  [[nodiscard]] unsigned int get_current_tick() const noexcept;
  [[nodiscard]] unsigned int days_elapsed() const noexcept;
  [[nodiscard]] std::chrono::sys_days actual_date() const noexcept;
  [[nodiscard]] unsigned int hour_of_day() const noexcept;
  [[nodiscard]] unsigned int weekday() const noexcept;
  [[nodiscard]] unsigned int minutes_per_tick() const noexcept;
};