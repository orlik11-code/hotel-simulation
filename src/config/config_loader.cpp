#include "config/config_loader.hpp"

#include "config/file_parse.hpp"
#include "config/sim_config_parse.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void merge_file(SimConfig &config, const std::string &path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("Cannot open configuration file: " + path);
  }

  std::string line;
  unsigned int line_no = 0;
  while (std::getline(in, line)) {
    ++line_no;
    if (file_parse::is_comment_or_empty(line)) {
      continue;
    }
    const auto [key, value] = file_parse::split_key_value(line, line_no);
    sim_config_parse::apply_key(config, key, value);
  }
}

} // namespace

SimConfig Config_loader::from_file(const std::string &path) {
  SimConfig config;
  merge_file(config, path);
  return config;
}

SimConfig Config_loader::from_argv(int argc, char *argv[],
                                   const SimConfig &base) {
  SimConfig config = base;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      print_usage(argc > 0 ? argv[0] : nullptr);
      std::exit(0);
    }
    if (arg == "--config" || arg == "--staff" || arg == "--hotel") {
      ++i;
      continue;
    }
    if (arg == "--tics-per-day" && i + 1 < argc) {
      config.ticsPerDay =
          file_parse::parse_positive(argv[++i], "tics_per_day");
    } else if (arg == "--minutes-per-tick" && i + 1 < argc) {
      config.minutesPerTick =
          file_parse::parse_positive(argv[++i], "minutes_per_tick");
    } else if (arg == "--max-ticks" && i + 1 < argc) {
      config.max_ticks = file_parse::parse_non_negative(argv[++i], "max_ticks");
    } else if (arg == "--event-probability" && i + 1 < argc) {
      config.event_probability =
          file_parse::parse_percent(argv[++i], "event_probability");
    }
  }

  return config;
}

SimConfig Config_loader::load(int argc, char *argv[], const SimConfig &base) {
  SimConfig config = base;

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--config" && i + 1 < argc) {
      merge_file(config, argv[++i]);
    }
  }

  return from_argv(argc, argv, config);
}

void Config_loader::print_usage(const char *program_name) {
  const char *name = program_name != nullptr ? program_name : "HotelApp";
  std::cout << "Usage: " << name << " [options]\n\n"
            << "Options:\n"
            << "  --hotel <path>          Load simulation + staff from one file (preferred)\n"
            << "  --config <path>         Load simulation settings only\n"
            << "  --staff <path>          Load staff only (same format as [worker]/[schedule])\n"
            << "  --tics-per-day <n>      Ticks per simulated day (default 24)\n"
            << "  --minutes-per-tick <n>  Model minutes per tick (default 60)\n"
            << "  --max-ticks <n>         Run n ticks (0 = interactive, default 0)\n"
            << "  --event-probability <n> True % chance per tick (1–100 roll), 0–30 (default 5)\n"
            << "  -h, --help              Show this message\n\n"
            << "Config file format (one key=value per line, # comments allowed):\n"
            << "  tics_per_day=24\n"
            << "  minutes_per_tick=60\n"
            << "  max_ticks=10\n"
            << "  event_probability=5\n"
            << "  date_start=2026-05-18\n";
}
