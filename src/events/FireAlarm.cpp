#include "events/FireAlarm.h"

FireAlarm::FireAlarm(Hotel &hotel) : HotelEvent(hotel) {}

FireAlarm::~FireAlarm() = default;

void FireAlarm::apply() {
  hotel.set_evacuation(true);
  hotel.set_power(false);
  hotel.set_actual_failure(true);
}

void FireAlarm::clear() {
  hotel.set_evacuation(false);
  hotel.set_power(true);
}

const char *FireAlarm::description() const { return "Fire alarm"; }
