#include <catch2/catch_test_macros.hpp>

#include "config/config_loader.hpp"

#include <chrono>
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
                    "minutes_per_tick=30\n"
                    "max_ticks=5\n"
                    "event_probability=2.5\n");

  const SimConfig cfg = Config_loader::from_file(path);
  REQUIRE(cfg.minutesPerTick == 30U);
  REQUIRE(cfg.max_ticks == 5U);
  REQUIRE(cfg.event_probability == 25U);

  std::remove(path.c_str());
}

TEST_CASE("Config_loader parses fractional event probability") {
  const std::string path = "test_config_fractional.conf";
  write_temp_config(path, "event_probability=0.5\n");

  const SimConfig cfg = Config_loader::from_file(path);
  REQUIRE(cfg.event_probability == 5U);

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
  write_temp_config(path, "minutes_per_tick=60\nmax_ticks=1\n");

  char arg0[] = "sim";
  char arg1[] = "--config";
  char arg2[] = "test_config_loader_argv.conf";
  char arg3[] = "--max-ticks";
  char arg4[] = "8";
  char *argv[] = {arg0, arg1, arg2, arg3, arg4};

  const SimConfig cfg = Config_loader::load(5, argv);
  REQUIRE(cfg.minutesPerTick == 60U);
  REQUIRE(cfg.max_ticks == 8U);

  std::remove(path.c_str());
}

TEST_CASE("Config_loader rejects negative values in file") {
  const std::string path = "test_config_loader_neg.conf";
  write_temp_config(path, "minutes_per_tick=-5\n");

  REQUIRE_THROWS_AS(Config_loader::from_file(path), std::invalid_argument);
  std::remove(path.c_str());
}

TEST_CASE("Config_loader rejects unknown keys") {
  const std::string path = "test_config_loader_bad.conf";
  write_temp_config(path, "unknown_key=1\n");

  REQUIRE_THROWS_AS(Config_loader::from_file(path), std::invalid_argument);
  std::remove(path.c_str());
}

TEST_CASE("Config_loader reads log_every_n_ticks and log_file") {
  const std::string path = "test_config_loader_log.conf";
  write_temp_config(path,
                    "log_every_n_ticks=5\n"
                    "log_file=run.log\n"
                    "log_file_every_n_ticks=10\n");

  const SimConfig cfg = Config_loader::from_file(path);
  REQUIRE(cfg.log_every_n_ticks == 5U);
  REQUIRE(cfg.log_file == "run.log");
  REQUIRE(cfg.log_file_every_n_ticks == 10U);

  std::remove(path.c_str());
}

TEST_CASE("Config_loader reads date_start as year_month_day") {
  const std::string path = "test_config_loader_date.conf";
  write_temp_config(path, "date_start=2026-05-18\n");

  const SimConfig cfg = Config_loader::from_file(path);
  REQUIRE(cfg.date_start.ok());
  REQUIRE(cfg.date_start.year() == std::chrono::year{2026});
  REQUIRE(cfg.date_start.month() == std::chrono::month{5});
  REQUIRE(cfg.date_start.day() == std::chrono::day{18});

  std::remove(path.c_str());
}

TEST_CASE("Config_loader rejects invalid date_start") {
  const std::string path = "test_config_loader_bad_date.conf";
  write_temp_config(path, "date_start=2026-02-30\n");

  REQUIRE_THROWS_AS(Config_loader::from_file(path), std::invalid_argument);
  std::remove(path.c_str());
}
