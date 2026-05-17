#include "Simulation.hpp"
#include "Hotel.h"
#include <iostream>

Simulation::Simulation(const SimConfig &new_sim_config) {
  config = new_sim_config;
  is_running = true;
  current_tick = 0;
  hotel = Hotel();
}

Hotel &Simulation::get_hotel() { return hotel; }

void Simulation::tick() {
  current_tick++;
  hotel.tick(config.minutesPerTick);
  
  std::cout << "[Czas] Minal 1 tick (" << config.minutesPerTick
            << " min). Obecny tick: " << current_tick << " (godz. "
            << hour_of_day() << ", dzien_tygodnia " << weekday() << ")\n";
}

unsigned int Simulation::get_current_tick() const noexcept {
  return current_tick;
}

unsigned int Simulation::hour_of_day() const noexcept {
  if (current_tick == 0U) {
    return 0U;
  }
  const unsigned int slot_in_day = (current_tick - 1U) % config.ticsPerDay;
  const unsigned int minutes_from_midnight =
      slot_in_day * config.minutesPerTick;
  return (minutes_from_midnight / 60U) % 24U;
}

unsigned int Simulation::weekday() const noexcept {
  if (current_tick == 0U) {
    return 0U;
  }
  return ((current_tick - 1U) / config.ticsPerDay) % 7U;
}

unsigned int Simulation::minutes_per_tick() const noexcept {
  return config.minutesPerTick;
}
