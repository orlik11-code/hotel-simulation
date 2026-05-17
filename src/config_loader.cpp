#include "config_loader.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

[[nodiscard]] std::string trim(std::string_view s) {
  const auto start = s.find_first_not_of(" \t\r\n");
  if (start == std::string_view::npos) {
    return {};
  }
  const auto end = s.find_last_not_of(" \t\r\n");
  return std::string(s.substr(start, end - start + 1));
}

[[nodiscard]] unsigned int parse_positive(const std::string &value,
                                          const std::string &field) {
  if (value.empty()) {
    throw std::invalid_argument("Empty value for " + field);
  }
  if (value[0] == '-') {
    throw std::invalid_argument("Values cannot be negative in " + field);
  }
  if (value == "0") {
    throw std::invalid_argument("Values cannot be 0 in " + field);
  }
  try {
    const unsigned long parsed = std::stoul(value);
    return static_cast<unsigned int>(parsed);
  } catch (const std::exception &) {
    throw std::invalid_argument("Invalid value for " + field + ": " + value);
  }
}

[[nodiscard]] unsigned int parse_non_negative(const std::string &value,
                                              const std::string &field) {
  if (value.empty()) {
    throw std::invalid_argument("Empty value for " + field);
  }
  if (value[0] == '-') {
    throw std::invalid_argument("Values cannot be negative in " + field);
  }
  try {
    return static_cast<unsigned int>(std::stoul(value));
  } catch (const std::exception &) {
    throw std::invalid_argument("Invalid value for " + field + ": " + value);
  }
}

void apply_key(SimConfig &config, const std::string &key,
               const std::string &value) {
  if (key == "tics_per_day") {
    config.ticsPerDay = parse_positive(value, "tics_per_day");
  } else if (key == "minutes_per_tick") {
    config.minutesPerTick = parse_positive(value, "minutes_per_tick");
  } else if (key == "max_ticks") {
    config.max_ticks = parse_non_negative(value, "max_ticks");
  } else {
    throw std::invalid_argument("Unknown config key: " + key);
  }
}

} // namespace

SimConfig Config_loader::from_file(const std::string &path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("Cannot open configuration file: " + path);
  }

  SimConfig config;
  std::string line;
  unsigned int line_no = 0;

  while (std::getline(in, line)) {
    ++line_no;
    line = trim(line);
    if (line.empty() || line[0] == '#') {
      continue;
    }

    const auto eq = line.find('=');
    if (eq == std::string::npos) {
      throw std::invalid_argument("Config line " + std::to_string(line_no) +
                                  ": expected key=value");
    }

    apply_key(config, trim(line.substr(0, eq)), trim(line.substr(eq + 1)));
  }

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
    if (arg == "--config") {
      ++i;
      continue;
    }
    if (arg == "--tics-per-day" && i + 1 < argc) {
      config.ticsPerDay = parse_positive(argv[++i], "tics_per_day");
    } else if (arg == "--minutes-per-tick" && i + 1 < argc) {
      config.minutesPerTick = parse_positive(argv[++i], "minutes_per_tick");
    } else if (arg == "--max-ticks" && i + 1 < argc) {
      config.max_ticks = parse_non_negative(argv[++i], "max_ticks");
    }
  }

  return config;
}

SimConfig Config_loader::load(int argc, char *argv[]) {
  SimConfig config;

  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--config" && i + 1 < argc) {
      config = from_file(argv[++i]);
    }
  }

  return from_argv(argc, argv, config);
}

void Config_loader::print_usage(const char *program_name) {
  const char *name = program_name != nullptr ? program_name : "HotelApp";
  std::cout << "Usage: " << name << " [options]\n\n"
            << "Options:\n"
            << "  --config <path>         Load simulation settings from a file\n"
            << "  --tics-per-day <n>      Ticks per simulated day (default 24)\n"
            << "  --minutes-per-tick <n>  Model minutes per tick (default 60)\n"
            << "  --max-ticks <n>         Run n ticks (0 = interactive, default 0)\n"
            << "  -h, --help              Show this message\n\n"
            << "Config file format (one key=value per line, # comments allowed):\n"
            << "  tics_per_day=24\n"
            << "  minutes_per_tick=60\n"
            << "  max_ticks=10\n";
}
