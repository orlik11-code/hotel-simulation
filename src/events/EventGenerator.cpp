//
// Created by orlik on 15.05.2026.
//

#include "events/EventGenerator.h"

#include "events/FireAlarm.h"
#include "events/PowerOutage.h"

#include <random>

// TODO: użyć random_chance(probability_event_happens) i random_chance(70U) z simulation_rng.

Hotel &EventGenerator::get_hotel() const { return this->hotel; }

EventGenerator::EventGenerator(Hotel &h, unsigned int p)
    : probability_event_happens(p), hotel(h), rd(), gen(rd()) {
  if (p > 300U) {
    throw std::invalid_argument("Probability cannot be greater than 30%");
  }
}

std::unique_ptr<HotelEvent> EventGenerator::decide_what_happens() {
  std::uniform_int_distribution<> roll(1, 1000);
  const unsigned int value = static_cast<unsigned int>(roll(gen));

  if (value <= probability_event_happens) {
    const unsigned int kind_roll = static_cast<unsigned int>(roll(gen));
    if (kind_roll <= 700U) {
      return std::make_unique<PowerOutage>(this->get_hotel());
    }
    return std::make_unique<FireAlarm>(this->get_hotel());
  }
  return nullptr;
}
