#pragma once

#include "simulation/SimConfig.hpp"

#include <string>

class Hotel;

/** Loads [simulation], [worker], [schedule], [room], [reservation], [reservation_guest]. */
class Hotel_loader {
public:
  [[nodiscard]] static SimConfig load(const std::string &path, Hotel &hotel);

  /** Reads {@code --hotel <path>} if present; otherwise returns default {@link SimConfig}. */
  [[nodiscard]] static SimConfig load_from_argv(int argc, char *argv[], Hotel &hotel);

  static void print_usage(const char *program_name = "HotelApp");
};
