#include <catch2/catch_test_macros.hpp>

#include "hotel/Hotel.h"
#include "rooms/SingleRoom.h"
#include "staff/Housekeeper.hpp"

#include <memory>

namespace {

void add_housekeeper_on_duty(Hotel &hotel) {
  auto keeper = std::make_unique<Housekeeper>(42U, "Zofia", "Sprzat", 12U, 6U, 5U);
  Shift day_shift{};
  day_shift.weekday = Weekday::monday;
  day_shift.start_hour = 8U;
  day_shift.end_hour = 16U;
  keeper->determine_schedule(Weekday::monday, day_shift);
  hotel.add_worker(std::move(keeper));
}

void add_dirty_room(Hotel &hotel, unsigned int room_number) {
  auto room = std::make_unique<SingleRoom>(18.0, room_number, 1U, 1U, 0U, false,
                                           false);
  room->make_dirty();
  hotel.addRoom(std::move(room));
}

} // namespace

TEST_CASE("Hotel assigns CleanRoom to dirty rooms during work hours") {
  Hotel hotel;
  add_housekeeper_on_duty(hotel);
  add_dirty_room(hotel, 101U);

  REQUIRE(hotel.dispatch_housekeeping(10U, 0U) == 1U);

  Staff *keeper = hotel.find_by_role(StaffRole::Housekeeper).front();
  REQUIRE(keeper != nullptr);
  REQUIRE(keeper->get_is_busy());
}

TEST_CASE("Hotel cleans room after housekeeper finishes task") {
  Hotel hotel;
  add_housekeeper_on_duty(hotel);

  auto room = std::make_unique<SingleRoom>(18.0, 202U, 1U, 1U, 0U, false, false);
  room->make_dirty();
  Room *room_ptr = room.get();
  hotel.addRoom(std::move(room));

  REQUIRE(hotel.dispatch_housekeeping(12U, 0U) == 1U);

  for (unsigned int i = 0U; i < 60U; ++i) {
    hotel.tick(1U);
    hotel.apply_completed_housekeeping();
  }

  Staff *keeper = hotel.find_by_role(StaffRole::Housekeeper).front();
  REQUIRE(keeper != nullptr);
  REQUIRE_FALSE(keeper->get_is_busy());
  REQUIRE_FALSE(room_ptr->is_to_clean());
}

TEST_CASE("Hotel skips housekeeping outside work hours") {
  Hotel hotel;
  add_housekeeper_on_duty(hotel);
  add_dirty_room(hotel, 303U);

  REQUIRE(hotel.dispatch_housekeeping(6U, 0U) == 0U);

  const std::vector<Staff *> keepers =
      hotel.find_by_role(StaffRole::Housekeeper);
  REQUIRE(keepers.size() == 1U);
  REQUIRE_FALSE(keepers.front()->get_is_busy());
}
