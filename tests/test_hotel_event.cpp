#include <catch2/catch_test_macros.hpp>

#include "events/FireAlarm.h"
#include "events/PowerOutage.h"
#include "hotel/Hotel.h"

TEST_CASE("PowerOutage apply and clear change hotel power and failure") {
  Hotel hotel;
  hotel.set_power(true);
  hotel.set_actual_failure(false);

  PowerOutage outage(hotel);
  outage.apply();
  REQUIRE_FALSE(hotel.get_power());
  REQUIRE(hotel.get_actual_failure());

  outage.clear();
  REQUIRE(hotel.get_power());
  REQUIRE(hotel.get_actual_failure());
}

TEST_CASE("FireAlarm apply and clear set evacuation and power") {
  Hotel hotel;
  hotel.set_power(true);
  hotel.set_evacuation(false);

  FireAlarm alarm(hotel);
  alarm.apply();
  REQUIRE_FALSE(hotel.get_power());
  REQUIRE(hotel.get_evacuation());
  REQUIRE(hotel.get_actual_failure());

  alarm.clear();
  REQUIRE(hotel.get_power());
  REQUIRE_FALSE(hotel.get_evacuation());
}
