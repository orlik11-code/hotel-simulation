#include <catch2/catch_test_macros.hpp>

#include "simulation/SimEventLog.hpp"
#include "simulation/Simulation.hpp"

TEST_CASE("Simulation fills event log map during run") {
  SimConfig cfg;
  cfg.ticsPerDay = 24U;
  cfg.minutesPerTick = 60U;
  cfg.event_probability = 0U;
  cfg.max_ticks = 2U;

  Simulation simulation(cfg);
  REQUIRE(simulation.get_event_log().empty());

  simulation.run();

  const SimEventLog &log = simulation.get_event_log();
  REQUIRE(log.contains(sim_event_category::symulacja));
  REQUIRE(log.contains(sim_event_category::czas));
  REQUIRE(log.at(sim_event_category::czas).size() == 2U);
  REQUIRE(log.at(sim_event_category::czas)[0].second == 1);
  REQUIRE(log.at(sim_event_category::czas)[1].second == 2);
}
