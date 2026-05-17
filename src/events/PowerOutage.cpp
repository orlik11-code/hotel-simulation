//
// Created by orlik on 15.05.2026.
//

#include "events/PowerOutage.h"

PowerOutage::PowerOutage(Hotel &h) : HotelEvent(h) { h.set_power(false); }

PowerOutage::~PowerOutage() { hotel.set_power(true); }