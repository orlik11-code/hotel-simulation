#include "simulation/Simulation.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace {

[[nodiscard]] unsigned int
weekday_index_from_sys_days(std::chrono::sys_days day) {
  const std::chrono::weekday wd{day};
  const unsigned int c = wd.c_encoding();
  if (c == 0U) {
    return 6U;
  }
  return c - 1U;
}

} // namespace

unsigned int Simulation::get_current_tick() const noexcept {
  return current_tick_;
}

unsigned int Simulation::days_elapsed() const noexcept {
  if (current_tick_ == 0U) {
    return 0U;
  }
  const unsigned long long total_minutes =
      static_cast<unsigned long long>(current_tick_ - 1U) *
      static_cast<unsigned long long>(config_.minutesPerTick);
  return static_cast<unsigned int>(total_minutes / 1440ULL);
}

std::chrono::sys_days Simulation::date_from_day_offset(
    unsigned int days_elapsed) const noexcept {
  return std::chrono::sys_days{config_.date_start} +
         std::chrono::days{static_cast<int>(days_elapsed)};
}

std::chrono::sys_days Simulation::actual_date() const noexcept {
  return date_from_day_offset(days_elapsed());
}

unsigned int Simulation::minutes_from_midnight() const noexcept {
  if (current_tick_ == 0U) {
    return 0U;
  }
  const unsigned long long total_minutes =
      static_cast<unsigned long long>(current_tick_ - 1U) *
      static_cast<unsigned long long>(config_.minutesPerTick);
  return static_cast<unsigned int>(total_minutes % 1440ULL);
}

unsigned int Simulation::hour_of_day() const noexcept {
  return (minutes_from_midnight() / 60U) % 24U;
}

unsigned int Simulation::minutes_of_hour() const noexcept {
  return minutes_from_midnight() % 60U;
}

unsigned int Simulation::weekday() const noexcept {
  if (!config_.date_start.ok()) {
    return days_elapsed() % 7U;
  }
  return weekday_index_from_sys_days(actual_date());
}

unsigned int Simulation::minutes_per_tick() const noexcept {
  return config_.minutesPerTick;
}

std::string Simulation::format_virtual_time() const {
  return format_virtual_time_for_tick(current_tick_);
}

std::string Simulation::format_virtual_time_for_tick(unsigned int tick) const {
  const unsigned long long total_minutes =
      (tick == 0U ? 0ULL : static_cast<unsigned long long>(tick - 1U)) *
      static_cast<unsigned long long>(config_.minutesPerTick);
  const unsigned int minute_of_day =
      static_cast<unsigned int>(total_minutes % 1440ULL);
  const unsigned int hour = (minute_of_day / 60U) % 24U;
  const unsigned int minute = minute_of_day % 60U;

  std::ostringstream out;
  if (config_.date_start.ok()) {
    const unsigned int day_offset =
        static_cast<unsigned int>(total_minutes / 1440ULL);
    const std::chrono::sys_days day =
        std::chrono::sys_days{config_.date_start} +
        std::chrono::days{static_cast<int>(day_offset)};
    const std::chrono::year_month_day ymd{day};
    out << std::setfill('0') << static_cast<int>(ymd.year()) << '-'
        << std::setw(2) << static_cast<unsigned>(ymd.month()) << '-'
        << std::setw(2) << static_cast<unsigned>(ymd.day());
  } else {
    out << "day-" << (total_minutes / 1440ULL);
  }
  out << ' ' << std::setw(2) << hour << ':' << std::setw(2) << minute;
  return out.str();
}