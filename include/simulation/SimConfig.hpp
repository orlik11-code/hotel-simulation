#pragma once

#include "meal/MealTypes.hpp"

#include <chrono>
#include <vector>

struct SimConfig {
  unsigned int minutesPerTick = 60;
  unsigned int max_ticks = 0;
  /** Event chance in tenths of a percent (50 = 5%, 5 = 0.5%); max 300 = 30%. */
  unsigned int event_probability = 50;
  /** Invalid until set from config (date_start=YYYY-MM-DD). */
  std::chrono::year_month_day date_start{};
  std::vector<MealServiceWindow> meal_windows;
  /** Chance 1–100 that a guest with food_included attends a given meal on a given day. */
  unsigned int meal_attendance_percent = 80U;
  /** How often to print [time] logs to console (1 = every tick). */
  unsigned int log_every_n_ticks = 1U;
  /** File export interval in ticks (0 = same as log_every_n_ticks). */
  unsigned int log_file_every_n_ticks = 0U;
  /** Empty = no log export after simulation ends. */
  std::string log_file;
};
