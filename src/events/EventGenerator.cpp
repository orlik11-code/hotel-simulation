//
// Created by orlik on 15.05.2026.
//

#include "events/EventGenerator.h"

#include "events/FireAlarm.h"
#include "events/PowerOutage.h"
#include "simulation/simulation_rng.hpp"

Hotel &EventGenerator::get_hotel() const { return this->hotel; }

EventGenerator::EventGenerator(Hotel &h, unsigned int p)
    : probability_event_happens(p), hotel(h) {
  if (p > 300U) {
    throw std::invalid_argument("Probability cannot be greater than 30%");
  }
}

std::unique_ptr<HotelEvent> EventGenerator::decide_what_happens() {
  const unsigned int value = random_unsigned(1U, 1000U);

  if (value <= probability_event_happens) {
    if (random_unsigned(1U, 1000U) <= 700U) {
      return std::make_unique<PowerOutage>(this->get_hotel());
    }
    return std::make_unique<FireAlarm>(this->get_hotel());
  }
  return nullptr;
}
