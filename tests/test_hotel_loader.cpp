#include <catch2/catch_test_macros.hpp>

#include "config/hotel_loader.hpp"
#include "hotel/Hotel.h"
#include "simulation/Simulation.hpp"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <string>

namespace {

void write_temp_hotel_file(const std::string &path, const std::string &body) {
  std::ofstream out(path);
  REQUIRE(out);
  out << body;
}

} // namespace

TEST_CASE("Hotel_loader reads simulation and staff sections") {
  const std::string path = "test_hotel_loader.conf";
  write_temp_hotel_file(path,
                        "[simulation]\n"
                        "minutes_per_tick=30\n"
                        "max_ticks=5\n"
                        "event_probability=3\n"
                        "date_start=2026-01-05\n"
                        "[worker]\n"
                        "uid=10\n"
                        "role=Chef\n"
                        "name=Ala\n"
                        "surname=Kuch\n"
                        "salary=12\n"
                        "salary_night=4\n"
                        "fatigue_rate=5\n"
                        "[schedule]\n"
                        "day=monday\n"
                        "start=8\n"
                        "end=12\n");

  Hotel hotel;
  const SimConfig cfg = Hotel_loader::load(path, hotel);

  REQUIRE(cfg.minutesPerTick == 30U);
  REQUIRE(cfg.max_ticks == 5U);
  REQUIRE(cfg.event_probability == 30U);
  REQUIRE(cfg.date_start.ok());
  REQUIRE(cfg.date_start == std::chrono::year_month_day{
                               std::chrono::year{2026}, std::chrono::month{1},
                               std::chrono::day{5}});
  REQUIRE(hotel.check_existance(10U));
  Staff *chef = hotel.find_by_uid(10U);
  REQUIRE(chef != nullptr);
  REQUIRE(chef->role() == StaffRole::Chef);
  REQUIRE(chef->is_on_shift(9U, 0U));
  REQUIRE_FALSE(chef->is_on_shift(13U, 0U));

  std::remove(path.c_str());
}

TEST_CASE("Hotel_loader staff-only file leaves default simulation config") {
  const std::string path = "test_hotel_loader_staff.conf";
  write_temp_hotel_file(path,
                        "[worker]\n"
                        "uid=3\n"
                        "role=Waiter\n"
                        "name=Bob\n"
                        "surname=Tray\n"
                        "salary=10\n"
                        "salary_night=5\n"
                        "fatigue_rate=3\n"
                        "[schedule]\n"
                        "day=friday\n"
                        "start=10\n"
                        "end=18\n");

  Hotel hotel;
  const SimConfig cfg = Hotel_loader::load(path, hotel);

  REQUIRE(cfg.minutesPerTick == 60U);
  REQUIRE(cfg.max_ticks == 0U);
  REQUIRE(hotel.check_existance(3U));

  std::remove(path.c_str());
}

TEST_CASE("Hotel_loader reads reservation check-in and check-out times") {
  const std::string path = "test_hotel_loader_reservation_times.conf";
  write_temp_hotel_file(path,
                        "[room]\n"
                        "type=Single\n"
                        "number=501\n"
                        "floor=1\n"
                        "area=20\n"
                        "[reservation]\n"
                        "room_number=501\n"
                        "check_in=2026-05-18\n"
                        "check_out=2026-05-20\n"
                        "hour_check_in=14\n"
                        "minute_check_in=15\n"
                        "hour_check_out=9\n"
                        "minute_check_out=45\n"
                        "food_included=false\n"
                        "food_price=0\n"
                        "[reservation_guest]\n"
                        "type=Business\n"
                        "name=Test\n"
                        "surname=Guest\n");

  Hotel hotel;
  (void)Hotel_loader::load(path, hotel);
  REQUIRE(hotel.get_reservations().size() == 1U);
  const Reservation *reservation = hotel.get_reservations().front();
  REQUIRE(reservation->get_hour_check_in() == 14);
  REQUIRE(reservation->get_minute_check_in() == 15);
  REQUIRE(reservation->get_hour_check_out() == 9);
  REQUIRE(reservation->get_minute_check_out() == 45);

  std::remove(path.c_str());
}
