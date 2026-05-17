#pragma once

#include "simulation/Simulation.hpp"

#include <string>

class Hotel;

/** Loads [simulation], [worker], and [schedule] sections from a single hotel file. */
class Hotel_loader {
public:
  [[nodiscard]] static SimConfig load(const std::string &path, Hotel &hotel);

  /** Reads {@code --hotel <path>} if present; otherwise returns default {@link SimConfig}. */
  [[nodiscard]] static SimConfig load_from_argv(int argc, char *argv[], Hotel &hotel);

  static void print_usage(const char *program_name = "HotelApp");
};
