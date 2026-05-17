//
// Created by orlik on 15.05.2026.
//

#include "events/EventGenerator.h"

#include "events/FireAlarm.h"
#include "events/PowerOutage.h"

#include <random>
Hotel &EventGenerator::get_hotel() const { return this->hotel; }

EventGenerator::EventGenerator(Hotel &h, unsigned int p)
    : probability_event_happens(p), hotel(h), rd(), gen(rd()) {
  if (p > 100) {
    throw std::invalid_argument("Probability cannot be greater than 100%");
  }
}

std::unique_ptr<HotelEvent> EventGenerator::decide_what_happens() {
  std::uniform_int_distribution<> distribution(0, 100);
  int random_p = distribution(gen);

  if (static_cast<unsigned int>(random_p) <= probability_event_happens) {
    random_p = distribution(gen);
    if (random_p <= 70) {
      return std::make_unique<PowerOutage>(this->get_hotel());
    } else {
      return std::make_unique<FireAlarm>(this->get_hotel());
    }
  }
  return nullptr;
}