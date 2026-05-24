#include "events/PowerOutage.h"

PowerOutage::PowerOutage(Hotel &h) : HotelEvent(h) {}

PowerOutage::~PowerOutage() = default;

void PowerOutage::apply() {
  hotel.set_power(false);
  hotel.set_actual_failure(true);
}

void PowerOutage::clear() { hotel.set_power(true); }

const char *PowerOutage::description() const { return "Awaria zasilania"; }
