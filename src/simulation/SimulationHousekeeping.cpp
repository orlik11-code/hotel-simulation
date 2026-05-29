#include "simulation/Simulation.hpp"

#include <sstream>

bool Simulation::should_run_daily_dirty(unsigned int hour_of_day,
                                      std::chrono::sys_days today) const {
  if (hour_of_day < hotel_.get_hotel_hour_start()) {
    return false;
  }
  if (last_daily_dirty_date_.has_value() &&
      *last_daily_dirty_date_ == today) {
    return false;
  }
  return true;
}

void Simulation::process_daily_dirty_rooms(std::chrono::sys_days today) {
  unsigned int marked = 0U;

  for (Reservation *reservation : hotel_.get_reservations()) {
    if (reservation == nullptr || reservation->get_is_cancelled()) {
      continue;
    }

    const bool active_stay = reservation->check_has_begun(today) &&
                             !reservation->check_resolved(today);
    const bool checkout_today = reservation->get_check_out() == today;

    if (!active_stay && !checkout_today) {
      continue;
    }

    if (hotel_.mark_room_dirty(reservation->get_room().get_number())) {
      ++marked;
    }
  }

  last_daily_dirty_date_ = today;
  std::ostringstream msg;
  msg << "Dzienny reset o " << hotel_.get_hotel_hour_start()
      << ":00 — oznaczono " << marked << " pokoi jako brudne.";
  log_event(sim_event_category::sprzatanie, msg.str());
}

void Simulation::tick_mark_daily_dirty_rooms() {
  const std::chrono::sys_days today = actual_date();
  if (!should_run_daily_dirty(hour_of_day(), today)) {
    return;
  }
  process_daily_dirty_rooms(today);
}

void Simulation::tick_dispatch_housekeeping(unsigned int hour,
                                            unsigned int weekday) {
  hotel_.apply_completed_housekeeping();
  if (hotel_.get_actual_failure()) {
    return;
  }

  const HousekeepingStats hk_stats =
      hotel_.collect_housekeeping_stats(hour, weekday);
  const unsigned int hk_assigned = hotel_.dispatch_housekeeping(hour, weekday);
  if (hk_stats.dirty_rooms == 0U && hk_assigned == 0U) {
    return;
  }

  std::ostringstream msg;
  msg << "brudne: " << hk_stats.dirty_rooms << ", w kolejce: "
      << hk_stats.need_assignment
      << ", pokojowki wolne: " << hk_stats.housekeepers_free
      << ", przypisano: " << hk_assigned;
  log_event(sim_event_category::sprzatanie, msg.str());
}
