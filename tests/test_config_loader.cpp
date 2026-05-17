#include <catch2/catch_test_macros.hpp>

#include "config_loader.hpp"

#include <cstdio>
#include <fstream>
#include <string>

namespace {

void write_temp_config(const std::string &path, const std::string &body) {
  std::ofstream out(path);
  REQUIRE(out);
  out << body;
}

} // namespace

TEST_CASE("Config_loader reads key/value file into SimConfig") {
  const std::string path = "test_config_loader.conf";
  write_temp_config(path,
                    "tics_per_day=12\n"
                    "minutes_per_tick=30\n"
                    "max_ticks=5\n");

  const SimConfig cfg = Config_loader::from_file(path);
  REQUIRE(cfg.ticsPerDay == 12U);
  REQUIRE(cfg.minutesPerTick == 30U);
  REQUIRE(cfg.max_ticks == 5U);

  std::remove(path.c_str());
}

TEST_CASE("Config_loader allows max_ticks zero in file") {
  const std::string path = "test_config_loader_zero.conf";
  write_temp_config(path, "max_ticks=0\n");

  const SimConfig cfg = Config_loader::from_file(path);
  REQUIRE(cfg.max_ticks == 0U);

  std::remove(path.c_str());
}

TEST_CASE("Config_loader argv overrides file values") {
  const std::string path = "test_config_loader_argv.conf";
  write_temp_config(path, "tics_per_day=24\nminutes_per_tick=60\nmax_ticks=1\n");

  char arg0[] = "sim";
  char arg1[] = "--config";
  char arg2[] = "test_config_loader_argv.conf";
  char arg3[] = "--max-ticks";
  char arg4[] = "8";
  char *argv[] = {arg0, arg1, arg2, arg3, arg4};

  const SimConfig cfg = Config_loader::load(5, argv);
  REQUIRE(cfg.ticsPerDay == 24U);
  REQUIRE(cfg.minutesPerTick == 60U);
  REQUIRE(cfg.max_ticks == 8U);

  std::remove(path.c_str());
}

TEST_CASE("Config_loader rejects negative values in file") {
  const std::string path = "test_config_loader_neg.conf";
  write_temp_config(path, "tics_per_day=-5\n");

  REQUIRE_THROWS_AS(Config_loader::from_file(path), std::invalid_argument);
  std::remove(path.c_str());
}

TEST_CASE("Config_loader rejects unknown keys") {
  const std::string path = "test_config_loader_bad.conf";
  write_temp_config(path, "unknown_key=1\n");

  REQUIRE_THROWS_AS(Config_loader::from_file(path), std::invalid_argument);
  std::remove(path.c_str());
}
