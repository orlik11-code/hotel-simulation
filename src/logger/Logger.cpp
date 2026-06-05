#include "logger/Logger.hpp"

#include <set>

Logger::Logger(const std::string &path, const Simulation &simulation,
               int frequency)
    : file_path(path), simulation(simulation), frequency(frequency) {
  log_file.open(file_path, std::ios::out);
  if (!log_file.is_open()) {
    throw std::runtime_error("Could not open log file: " + file_path);
  }
  if (frequency <= 0) {
    throw std::invalid_argument("Frequency must be a positive integer");
  }
}

void Logger::prepare_log(int tick_start, int tick_end,
                         const std::set<std::string> &categories) {
  const bool log_all = categories.empty();
  int act_tick = tick_start;
  const SimEventLog &event_log = simulation.get_event_log();

  while (act_tick <= tick_end) {
    for (const auto &[category, records] : event_log) {
      if (!log_all && categories.find(category) == categories.end()) {
        continue;
      }

      auto it =
          std::lower_bound(records.begin(), records.end(), act_tick,
                           [&](const SimEventRecord &record, int tick_to_cmp) {
                             return record.second < tick_to_cmp;
                           });

      while (it != records.end() && it->second == act_tick) {
        log_file << '[' << simulation.format_virtual_time_for_tick(
                              static_cast<unsigned int>(act_tick))
                 << "] [" << category << "] " << it->first << '\n';
        log_file.flush();
        ++it;
      }
    }
    act_tick += frequency;
  }
}

void Logger::set_frequency(int new_frequency) {
  if (new_frequency <= 0) {
    throw std::invalid_argument("Frequency must be a positive integer");
  }
  frequency = new_frequency;
}

void Logger::set_file_path(const std::string &new_path) {
  log_file.close();
  file_path = new_path;
  log_file.open(file_path, std::ios::out);
  if (!log_file.is_open()) {
    throw std::runtime_error("Could not open log file: " + file_path);
  }
}
