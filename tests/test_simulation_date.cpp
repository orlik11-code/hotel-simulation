#include <catch2/catch_test_macros.hpp>

#include "simulation/Simulation.hpp"

TEST_CASE("Simulation weekday follows date_start from config") {
  SimConfig cfg;
  cfg.ticsPerDay = 24U;
  cfg.minutesPerTick = 60U;
  cfg.event_probability = 0U;
  cfg.date_start = std::chrono::year_month_day{
      std::chrono::year{2026}, std::chrono::month{5},
      std::chrono::day{18}}; // Monday

  Simulation simulation(cfg);
  REQUIRE(simulation.weekday() == 0U); // Monday before first tick

  simulation.tick();
  REQUIRE(simulation.weekday() == 0U);

  cfg.ticsPerDay = 1U;
  simulation.set_config(cfg);
  for (unsigned int i = 0U; i < 7U; ++i) {
    simulation.tick();
  }
  // 8 ticks total on config with 1 tick/day: day index 7 -> next Monday
  REQUIRE(simulation.weekday() == 0U);
}

TEST_CASE("Simulation days_elapsed and actual_date") {
  SimConfig cfg;
  cfg.ticsPerDay = 24U;
  cfg.minutesPerTick = 60U;
  cfg.event_probability = 0U;
  cfg.date_start = std::chrono::year_month_day{
      std::chrono::year{2026}, std::chrono::month{5},
      std::chrono::day{18}};

  Simulation simulation(cfg);
  REQUIRE(simulation.days_elapsed() == 0U);
  REQUIRE(simulation.actual_date() ==
          std::chrono::sys_days{cfg.date_start});

  for (unsigned int i = 0U; i < 24U; ++i) {
    simulation.tick();
  }
  REQUIRE(simulation.days_elapsed() == 0U);
  REQUIRE(simulation.actual_date() ==
          std::chrono::sys_days{cfg.date_start});

  simulation.tick();
  REQUIRE(simulation.days_elapsed() == 1U);
  REQUIRE(simulation.actual_date() ==
          std::chrono::sys_days{cfg.date_start} + std::chrono::days{1});
}
