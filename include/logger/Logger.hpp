#pragma once
#include "../simulation/Simulation.hpp"
#include <cstdio>
#include <fstream>
#include <set>
#include <string>

class Logger {
private:
  std::string file_path;
  std::ofstream log_file;
  Simulation &simulation;
  int frequency;

public:
  Logger(const std::string &path, Simulation &simulation, int frequency = 1);
  void prepare_log(int tick_start, int tick_end,
                   std::set<std::string> categories);
  ~Logger() = default;

  void set_frequency(int new_frequency);
  void set_file_path(const std::string &new_path);
};