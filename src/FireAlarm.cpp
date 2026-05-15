//
// Created by orlik on 15.05.2026.
//

#include "../include/FireAlarm.h"

FireAlarm::FireAlarm(Hotel &hotel) : HotelEvent(hotel) {
  hotel.set_evacuation(true);
  hotel.set_power(false);
}

FireAlarm::~FireAlarm() {
  hotel.set_evacuation(false);
  hotel.set_power(true);
}
