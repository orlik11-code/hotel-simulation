#include "../include/doctest.h"
#include "../include/logger/Logger.hpp"
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
  Simulation sim(config);
  Logger logger("output.log", sim, 1);
  std::set<std::string> categories = {"event"};

  SUBCASE("Empty range") {
    logger.prepare_log(100, 50, categories);
    std::ifstream file("output.log");
    std::string line;
    CHECK_FALSE(std::getline(file, line));
  }

  SUBCASE("Binary search - exact tick match") {
    logger.prepare_log(10, 10, categories);
    std::ifstream file("output.log");
    std::string line;
    bool found = false;
    while (std::getline(file, line)) {
      if (line.find("[10]") != std::string::npos) {
        found = true;
        break;
      }
    }
    CHECK(found);
  }

  SUBCASE("Binary search - non-existent tick") {
    logger.prepare_log(999, 999, categories);
    std::ifstream file("output.log");
    std::string line;
    CHECK_FALSE(std::getline(file, line));
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