#include <catch2/catch_test_macros.hpp>

#include "hotel/Hotel.h"
#include "staff/staff_loader.hpp"

#include <cstdio>
#include <fstream>
#include <string>

namespace {

void write_temp_staff_file(const std::string &path, const std::string &body) {
  std::ofstream out(path);
  REQUIRE(out);
  out << body;
}

} // namespace

TEST_CASE("Staff_loader loads worker and schedule sections") {
  const std::string path = "test_staff_loader.conf";
  write_temp_staff_file(path,
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
  Staff_loader::load_into(hotel, path);

  REQUIRE(hotel.check_existance(10U));
  Staff *chef = hotel.find_by_uid(10U);
  REQUIRE(chef != nullptr);
  REQUIRE(chef->role() == StaffRole::Chef);
  REQUIRE(chef->is_on_shift(9U, 0U));
  REQUIRE_FALSE(chef->is_on_shift(13U, 0U));

  std::remove(path.c_str());
}
