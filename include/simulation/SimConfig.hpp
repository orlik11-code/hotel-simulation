#pragma once

#include "meal/MealTypes.hpp"

#include <chrono>
#include <vector>

struct SimConfig {
  unsigned int minutesPerTick = 60;
  unsigned int max_ticks = 0;
  /** Szansa awarii w setnych procenta (50 = 5%, 5 = 0.5%); max 300 = 30%. */
  unsigned int event_probability = 50;
  /** Invalid until set from config (date_start=YYYY-MM-DD). */
  std::chrono::year_month_day date_start{};
  std::vector<MealServiceWindow> meal_windows;
  /** Szansa 1–100, że gość z food_included przyjdzie na dany posiłek w danym dniu. */
  unsigned int meal_attendance_percent = 80U;
  /** Co ile ticków wypisywać log [czas] na konsolę (1 = co tick). */
  unsigned int log_every_n_ticks = 1U;
  /** Co ile ticków zapisywać do pliku (0 = jak log_every_n_ticks). */
  unsigned int log_file_every_n_ticks = 0U;
  /** Pusty = brak eksportu po zakończeniu symulacji. */
  std::string log_file;
};
