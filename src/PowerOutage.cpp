//
// Created by orlik on 15.05.2026.
//

#include "../include/PowerOutage.h"

PowerOutage::PowerOutage(Hotel &h) : hotel(h), active(true) {
  h.set_power(false);
}

PowerOutage::~PowerOutage() {
  if (active) {
    hotel.set_power(true);
  }
}