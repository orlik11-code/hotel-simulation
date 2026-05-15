//
// Created by orlik on 15.05.2026.
//

#include "../include/FireAlarm.h"

FireAlarm::FireAlarm(Hotel &hotel) : hotel(hotel), active(true) {
  hotel.set_evacuation(true);
  hotel.set_power(false);
}

FireAlarm::~FireAlarm() {
  if (active) {
    hotel.set_evacuation(false);
    hotel.set_power(true);
  }
}
