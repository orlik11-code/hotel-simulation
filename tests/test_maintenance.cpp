#include <catch2/catch_test_macros.hpp>

#include "hotel/Hotel.h"
#include "staff/MaintenanceWorker.hpp"
#include "staff/types.hpp"

#include <memory>

namespace {

void add_maintenance_on_duty(Hotel &hotel, unsigned int uid) {
  auto worker =
      std::make_unique<MaintenanceWorker>(uid, "Jan", "Naprawa", 10U, 5U, 4U);
  Shift day_shift{};
  day_shift.weekday = Weekday::monday;
  day_shift.start_hour = 8U;
  day_shift.end_hour = 16U;
  worker->determine_schedule(Weekday::monday, day_shift);
  hotel.add_worker(std::move(worker));
}

} // namespace

TEST_CASE("Hotel clears failure after maintenance worker finishes FixFailure") {
  Hotel hotel;
  add_maintenance_on_duty(hotel, 77U);
  hotel.set_actual_failure(true);

  REQUIRE(hotel.try_assign_maintenance(10U, 0U));
  REQUIRE(hotel.has_pending_maintenance());
  REQUIRE(hotel.get_actual_failure());

  for (unsigned int i = 0U; i < 60U; ++i) {
    hotel.tick(1U);
    hotel.apply_completed_maintenance();
    if (!hotel.get_actual_failure()) {
      break;
    }
  }

  REQUIRE_FALSE(hotel.get_actual_failure());
  REQUIRE_FALSE(hotel.has_pending_maintenance());
}

TEST_CASE("Hotel does not assign second maintenance while pending") {
  Hotel hotel;
  add_maintenance_on_duty(hotel, 88U);
  hotel.set_actual_failure(true);

  REQUIRE(hotel.try_assign_maintenance(10U, 0U));
  REQUIRE_FALSE(hotel.try_assign_maintenance(10U, 0U));
  REQUIRE(hotel.has_pending_maintenance());
}
