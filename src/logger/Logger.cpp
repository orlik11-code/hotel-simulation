#include "../../include/logger/Logger.hpp"
#include "../../include/simulation/Simulation.hpp"
#include <set>

Logger::Logger(const std::string &path, Simulation &simulation, int frequency)
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
                         std::set<std::string> categories) {
  int act_tick = tick_start;
  SimEventLog event_log = simulation.get_event_log();
  while (act_tick <= tick_end) {
    for (const auto &[category, records] : event_log) {
      if (categories.find(category) == categories.end()) {
        continue;
      }

      auto it =
          std::lower_bound(records.begin(), records.end(), act_tick,
                           [&](const SimEventRecord &record, int tick_to_cmp) {
                             return record.second < tick_to_cmp;
                           });

      while (it != records.end() && it->second == act_tick) {
        // log_file << "[" << simulation.time_from_tick(act_tick) << "] [" <<
        // category << "]:  " << it->first << "\n"; W Simulation brakuje
        // funkcji, która konwertuje tick na czas, więc na razie logujemy tylko
        // tick, ale istnieje opcja rozszerzenia.
        log_file << "[" << it->second << "] [" << category
                 << "]:  " << it->first << "\n";
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