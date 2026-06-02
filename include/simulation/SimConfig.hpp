#pragma once

#include "meal/MealTypes.hpp"

#include <chrono>
#include <vector>

struct SimConfig {
  unsigned int minutesPerTick = 60;
  unsigned int max_ticks = 0;
  /** True % chance per tick (1–100 roll); max 30 in config. */
  unsigned int event_probability = 5;
  /** Invalid until set from config (date_start=YYYY-MM-DD). */
  std::chrono::year_month_day date_start{};
  std::vector<MealServiceWindow> meal_windows;
  /** Szansa 1–100, że gość z food_included przyjdzie na dany posiłek w danym dniu. */
  unsigned int meal_attendance_percent = 80U;
};
