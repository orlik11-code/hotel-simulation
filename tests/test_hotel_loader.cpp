#include <catch2/catch_test_macros.hpp>

#include "config/hotel_loader.hpp"
#include "hotel/Hotel.h"

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
                        "tics_per_day=12\n"
                        "minutes_per_tick=30\n"
                        "max_ticks=5\n"
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

  REQUIRE(cfg.ticsPerDay == 12U);
  REQUIRE(cfg.minutesPerTick == 30U);
  REQUIRE(cfg.max_ticks == 5U);
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

  REQUIRE(cfg.ticsPerDay == 24U);
  REQUIRE(cfg.minutesPerTick == 60U);
  REQUIRE(cfg.max_ticks == 0U);
  REQUIRE(hotel.check_existance(3U));

  std::remove(path.c_str());
}
