#include "config/sim_config_parse.hpp"

#include "config/file_parse.hpp"

#include <stdexcept>

namespace sim_config_parse {

void apply_key(SimConfig &config, const std::string &key,
               const std::string &value) {
  if (key == "minutes_per_tick") {
    config.minutesPerTick =
        file_parse::parse_positive(value, "minutes_per_tick");
  } else if (key == "max_ticks") {
    config.max_ticks = file_parse::parse_non_negative(value, "max_ticks");
  } else if (key == "event_probability") {
    config.event_probability =
        file_parse::parse_percent(value, "event_probability");
  } else if (key == "date_start") {
    config.date_start = file_parse::parse_date(value, "date_start");
  } else if (key == "meal_attendance_percent") {
    const unsigned int percent =
        file_parse::parse_positive(value, "meal_attendance_percent");
    if (percent > 100U) {
      throw std::invalid_argument("meal_attendance_percent cannot exceed 100");
    }
    config.meal_attendance_percent = percent;
  } else if (key == "log_every_n_ticks") {
    config.log_every_n_ticks =
        file_parse::parse_positive(value, "log_every_n_ticks");
  } else if (key == "log_file_every_n_ticks") {
    config.log_file_every_n_ticks =
        file_parse::parse_non_negative(value, "log_file_every_n_ticks");
  } else if (key == "log_file") {
    config.log_file = value;
  } else {
    throw std::invalid_argument("Unknown simulation key: " + key);
  }
}

} // namespace sim_config_parse
