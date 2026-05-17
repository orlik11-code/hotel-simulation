#pragma once
#include "hotel/Hotel.h"

struct SimConfig {
  unsigned int ticsPerDay = 24;
  unsigned int minutesPerTick = 60;
  unsigned int max_ticks = 0;
};

class Simulation {
private:
  Hotel hotel;
  SimConfig config;
  unsigned int current_tick;
  bool is_running;

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