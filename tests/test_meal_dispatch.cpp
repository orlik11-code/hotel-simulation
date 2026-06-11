#include <catch2/catch_test_macros.hpp>

#include "config/hotel_loader.hpp"
#include "simulation/Simulation.hpp"

#include <fstream>

namespace {

void write_temp_hotel(const std::string &path) {
  std::ofstream out(path);
  REQUIRE(out);
  out << "[simulation]\n"
         "minutes_per_tick=60\n"
         "max_ticks=24\n"
         "event_probability=0\n"
         "date_start=2026-05-18\n"
         "meal_attendance_percent=100\n"
         "[room]\n"
         "type=Single\n"
         "number=201\n"
         "floor=1\n"
         "area=20\n"
         "[reservation]\n"
         "room_number=201\n"
         "check_in=2026-05-18\n"
         "check_out=2026-05-22\n"
         "food_included=true\n"
         "food_price=10\n"
         "[reservation_guest]\n"
         "type=VIP\n"
         "name=A\n"
         "surname=B\n"
         "[worker]\n"
         "uid=40\n"
         "role=Chef\n"
         "name=C\n"
         "surname=H\n"
         "salary=10\n"
         "salary_night=5\n"
         "fatigue_rate=1\n"
         "[schedule]\n"
         "day=monday\n"
         "start=7\n"
         "end=13\n"
         "[worker]\n"
         "uid=41\n"
         "role=Waiter\n"
         "name=W\n"
         "surname=T\n"
         "salary=10\n"
         "salary_night=5\n"
         "fatigue_rate=1\n"
         "[schedule]\n"
         "day=monday\n"
         "start=7\n"
         "end=13\n"
         "[meal_service]\n"
         "type=breakfast\n"
         "start_hour=8\n"
         "end_hour=10\n";
}

} // namespace

TEST_CASE("Simulation logs restaurant activity for food_included stay") {
  const std::string path = "test_meal_dispatch.conf";
  write_temp_hotel(path);

  Simulation simulation(SimConfig{});
  const SimConfig cfg =
      Hotel_loader::load(path, simulation.get_hotel());
  REQUIRE(cfg.meal_windows.size() == 1U);
  simulation.set_config(cfg);
  simulation.set_live_console_output(false);
  simulation.run();

  const SimEventLog &log = simulation.get_event_log();
  REQUIRE(log.contains(sim_event_category::restaurant));
  REQUIRE_FALSE(log.at(sim_event_category::restaurant).empty());

  std::remove(path.c_str());
}
