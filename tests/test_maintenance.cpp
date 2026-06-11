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
    hotel.tick(1U, 10U, 0U);
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

TEST_CASE("Hotel reports maintenance when a shift is on duty") {
  Hotel hotel;
  add_maintenance_on_duty(hotel, 99U);

  REQUIRE(hotel.has_maintenance_on_shift(10U, 0U));
  REQUIRE_FALSE(hotel.has_maintenance_on_shift(3U, 0U));
}

TEST_CASE("Three maintenance shifts cover morning, day and evening") {
  Hotel hotel;

  auto night = std::make_unique<MaintenanceWorker>(1U, "Jan", "Fix", 10U, 5U, 4U);
  Shift night_shift{};
  night_shift.weekday = Weekday::monday;
  night_shift.start_hour = 0U;
  night_shift.end_hour = 8U;
  night->determine_schedule(Weekday::monday, night_shift);
  hotel.add_worker(std::move(night));

  auto day = std::make_unique<MaintenanceWorker>(2U, "Oskar", "Naprawa", 10U, 5U, 4U);
  Shift day_shift{};
  day_shift.weekday = Weekday::monday;
  day_shift.start_hour = 8U;
  day_shift.end_hour = 16U;
  day->determine_schedule(Weekday::monday, day_shift);
  hotel.add_worker(std::move(day));

  auto evening = std::make_unique<MaintenanceWorker>(3U, "Wiktor", "Klucz", 10U, 5U, 4U);
  Shift evening_shift{};
  evening_shift.weekday = Weekday::monday;
  evening_shift.start_hour = 16U;
  evening_shift.end_hour = 24U;
  evening->determine_schedule(Weekday::monday, evening_shift);
  hotel.add_worker(std::move(evening));

  REQUIRE(hotel.has_maintenance_on_shift(3U, 0U));
  REQUIRE(hotel.has_maintenance_on_shift(10U, 0U));
  REQUIRE(hotel.has_maintenance_on_shift(20U, 0U));
  REQUIRE(hotel.has_maintenance_on_shift(23U, 0U));
}

TEST_CASE("Hotel balance decreases when maintenance completes emergency call") {
  Hotel hotel;
  add_maintenance_on_duty(hotel, 55U);
  hotel.change_balance(10'000);
  hotel.set_actual_failure(true);

  REQUIRE(hotel.try_assign_maintenance(10U, 0U));

  for (unsigned int i = 0U; i < 60U; ++i) {
    hotel.tick(1U, 10U, 0U);
    hotel.apply_completed_maintenance();
    if (!hotel.get_actual_failure()) {
      break;
    }
  }

  REQUIRE_FALSE(hotel.get_actual_failure());
  Staff *keeper = hotel.find_by_role(StaffRole::MaintenanceWorker).front();
  REQUIRE(keeper != nullptr);
  REQUIRE(keeper->get_to_pay() >= 500U);
  REQUIRE(hotel.get_balance() ==
          10'000 - static_cast<int>(keeper->get_to_pay()));
}
