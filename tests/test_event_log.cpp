#include <catch2/catch_test_macros.hpp>

#include "simulation/SimEventLog.hpp"
#include "simulation/Simulation.hpp"

TEST_CASE("Simulation fills event log map during run") {
  SimConfig cfg;
  cfg.minutesPerTick = 60U;
  cfg.event_probability = 0U;
  cfg.max_ticks = 2U;

  Simulation simulation(cfg);
  simulation.set_live_console_output(false);
  REQUIRE(simulation.get_event_log().empty());

  simulation.run();

  const SimEventLog &log = simulation.get_event_log();
  REQUIRE(log.contains(sim_event_category::symulacja));
  REQUIRE(log.contains(sim_event_category::czas));
  REQUIRE(log.at(sim_event_category::czas).size() == 2U);
  REQUIRE(log.at(sim_event_category::czas)[0].second == 1);
  REQUIRE(log.at(sim_event_category::czas)[1].second == 2);
}

TEST_CASE("log_every_n_ticks throttles console but keeps full event log") {
  SimConfig cfg;
  cfg.minutesPerTick = 60U;
  cfg.event_probability = 0U;
  cfg.max_ticks = 6U;
  cfg.log_every_n_ticks = 2U;

  Simulation simulation(cfg);
  simulation.set_live_console_output(true);
  simulation.run();

  const SimEventLog &log = simulation.get_event_log();
  REQUIRE(log.at(sim_event_category::czas).size() == 6U);
  REQUIRE(log.at(sim_event_category::czas)[1].first.find("2 ticks elapsed") !=
          std::string::npos);
}
