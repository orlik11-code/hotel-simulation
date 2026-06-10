//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_HOTEL_SYSTEM_MANAGEMENT_EVENTGENERATOR_H
#define PROI_HOTEL_SYSTEM_MANAGEMENT_EVENTGENERATOR_H

#include "events/HotelEvent.h"
#include "hotel/Hotel.h"

class EventGenerator {
public:
  EventGenerator(Hotel &h, unsigned int probability_event_happens);
  ~EventGenerator() = default;
  std::unique_ptr<HotelEvent> decide_what_happens();
  Hotel &get_hotel() const;

private:
  unsigned int probability_event_happens;
  Hotel &hotel;
};

#endif // PROI_HOTEL_SYSTEM_MANAGEMENT_EVENTGENERATOR_H
