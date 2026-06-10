#include "../include/doctest.h"
#include "../include/logger/Logger.hpp"
#include "../include/simulation/SimEventLog.hpp"
#include "../include/simulation/Simulation.hpp"

TEST_CASE("Logger initialization and configuration") {
  SimConfig config;
  Simulation sim(config);

  SUBCASE("Constructor validation") {
    CHECK_THROWS_AS(Logger("test.log", sim, 0), std::invalid_argument);
    CHECK_THROWS_AS(Logger("test.log", sim, -1), std::invalid_argument);
    CHECK_NOTHROW(Logger("test.log", sim, 1));
  }

  SUBCASE("Frequency modification") {
    Logger logger("test.log", sim, 5);
    logger.set_frequency(10);
    CHECK_THROWS_AS(logger.set_frequency(0), std::invalid_argument);
  }
}

TEST_CASE("Logger log generation logic") {
  SimConfig config;
  config.event_probability = 0; // Disable random events for predictable testing

  Simulation sim(config);

  for (int i = 0; i < 15; ++i) {
    sim.tick();
  }

  Logger logger("output.log", sim, 1);
  std::set<std::string> categories = {"czas"};

  SUBCASE("Empty range") {
    logger.prepare_log(100, 200, categories);
    std::ifstream file("output.log");
    std::string line;
    CHECK_FALSE(static_cast<bool>(std::getline(file, line)));
  }

  SUBCASE("Binary search - exact tick match") {
    logger.prepare_log(10, 10, categories);
    std::ifstream file("output.log");
    std::string line;
    bool found = false;
    while (std::getline(file, line)) {
      if (line.find("Current tick: 10") != std::string::npos) {
        found = true;
        break;
      }
    }
    CHECK(found);
  }

  SUBCASE("Exports one line per stored record in tick range") {
    const SimEventLog &log = sim.get_event_log();
    REQUIRE(log.contains("czas"));
    const auto &records = log.at("czas");
    REQUIRE(records.size() == 15U);

    logger.prepare_log(1, 15, categories);

    std::ifstream file("output.log");
    std::string line;
    unsigned int line_count = 0U;
    while (std::getline(file, line)) {
      if (!line.empty()) {
        ++line_count;
      }
    }
    CHECK(line_count == records.size());
  }
}

TEST_CASE("File management") {
  SimConfig config;
  Simulation sim(config);
  Logger logger("first.log", sim, 1);

  CHECK_NOTHROW(logger.set_file_path("second.log"));

  std::ofstream test_out("second.log");
  CHECK(test_out.is_open());
}