#pragma once

#include "simulation/SimConfig.hpp"

#include <string>

class Config_loader {
public:
  [[nodiscard]] static SimConfig from_file(const std::string &path);

  [[nodiscard]] static SimConfig from_argv(int argc, char *argv[],
                                             const SimConfig &base = {});

  [[nodiscard]] static SimConfig load(int argc, char *argv[],
                                      const SimConfig &base = {});

  static void print_usage(const char *program_name = "HotelApp");
};
