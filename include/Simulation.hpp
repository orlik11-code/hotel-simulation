#pragma once
#include "Hotel.h"

struct SimConfig {
  /** Liczba ticków na jedną „dobę” modelową (np. 24 albo 72). */
  unsigned int ticsPerDay = 24;
  /**
   * Ile minut rzeczywistego czasu modelu mija w jednym ticku (np. 60 = 1 h, 20
   * = ⅓ h). Typowo: ticsPerDay * minutesPerTick == 24 * 60 (pełna doba).
   */
  unsigned int minutesPerTick = 60;
};

class Simulation {
private:
  Hotel hotel;
  SimConfig config;
  unsigned int current_tick;
  bool is_running;

public:
  Simulation(const SimConfig &sim_config);

  void run();
  void tick();

  Hotel &get_hotel();

  [[nodiscard]] unsigned int get_current_tick() const noexcept;
  [[nodiscard]] unsigned int hour_of_day() const noexcept;
  [[nodiscard]] unsigned int weekday() const noexcept;
  [[nodiscard]] unsigned int minutes_per_tick() const noexcept;
};