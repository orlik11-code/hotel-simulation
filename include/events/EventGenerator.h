//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_HOTEL_SYSTEM_MANAGEMENT_EVENTGENERATOR_H
#define PROI_HOTEL_SYSTEM_MANAGEMENT_EVENTGENERATOR_H

#include "events/HotelEvent.h"
#include "hotel/Hotel.h"

#include <random>

// TODO: przejść na simulation_rng.hpp / random_chance zamiast własnego rd+gen.

class EventGenerator {
public:
  EventGenerator(Hotel &h, unsigned int probability_event_happens);
  ~EventGenerator() = default;
  std::unique_ptr<HotelEvent> decide_what_happens();
  Hotel &get_hotel() const;

private:
  unsigned int probability_event_happens;
  Hotel &hotel;
  std::random_device rd;
  std::mt19937 gen;
};

#endif // PROI_HOTEL_SYSTEM_MANAGEMENT_EVENTGENERATOR_H
