#pragma once

#include "events/EventGenerator.h"
#include "hotel/Hotel.h"

#include <memory>

struct SimConfig {
  unsigned int ticsPerDay = 24;
  unsigned int minutesPerTick = 60;
  unsigned int max_ticks = 0;
  /** True % chance per tick (1–100 roll); max 30 in config. */
  unsigned int event_probability = 5;
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

public:
  Simulation(const SimConfig &sim_config);

  void set_config(const SimConfig &sim_config);

  void run();
  void tick();

  Hotel &get_hotel();

  [[nodiscard]] unsigned int get_current_tick() const noexcept;
  [[nodiscard]] unsigned int hour_of_day() const noexcept;
  [[nodiscard]] unsigned int weekday() const noexcept;
  [[nodiscard]] unsigned int minutes_per_tick() const noexcept;
};