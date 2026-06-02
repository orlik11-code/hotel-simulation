#include "simulation/Simulation.hpp"

#include <stdexcept>
#include <string>

namespace {

constexpr unsigned int kFailureDestroyAfterMinutes = 40U;
constexpr unsigned int kNoShowGraceMinutes = 120U;

} // namespace

void Simulation::try_fix_failure() {
  if (hotel_.has_pending_maintenance()) {
    time_with_fail_ += config_.minutesPerTick;
    if (time_with_fail_ >= kFailureDestroyAfterMinutes) {
      throw std::runtime_error("Hotel has been destroyed");
    }
    return;
  }

  if (hotel_.try_assign_maintenance(hour_of_day(), weekday())) {
    log_event(sim_event_category::awaria,
              "Przypisano konserwatora do naprawy awarii.");
    return;
  }

  time_with_fail_ += config_.minutesPerTick;
  if (time_with_fail_ >= kFailureDestroyAfterMinutes) {
    throw std::runtime_error("Hotel has been destroyed");
  }
}

void Simulation::tick_resolve_maintenance() {
  const bool had_failure = hotel_.get_actual_failure();
  hotel_.apply_completed_maintenance();
  if (!had_failure || hotel_.get_actual_failure()) {
    return;
  }

  time_with_fail_ = 0U;
  if (active_event_ != nullptr) {
    active_event_->clear();
    active_event_.reset();
  }
  log_event(sim_event_category::awaria, "Awaria naprawiona.");
}

void Simulation::tick_process_failures_and_random_events() {
  if (hotel_.get_actual_failure()) {
    try_fix_failure();
    return;
  }

  if (std::unique_ptr<HotelEvent> event =
          event_generator_->decide_what_happens()) {
    active_event_ = std::move(event);
    active_event_->apply();
    log_event(sim_event_category::awaria,
              std::string(active_event_->description()) + " (szansa " +
                  std::to_string(config_.event_probability) + "%).");
    try_fix_failure();
  }
}

void Simulation::schedule_cancellation(Reservation *reservation) {
  if (reservation == nullptr || reservation->get_is_cancelled()) {
    return;
  }
  for (Reservation *pending : pending_cancellations_) {
    if (pending == reservation) {
      return;
    }
  }
  pending_cancellations_.push_back(reservation);
}

void Simulation::tick_evaluate_hotel_cancellations(unsigned int hour,
                                                   unsigned int minute) {
  const std::chrono::sys_days today = actual_date();

  if (hotel_.get_evacuation()) {
    for (Reservation *reservation : hotel_.get_reservations()) {
      if (reservation == nullptr || reservation->get_is_cancelled()) {
        continue;
      }
      if (reservation->check_has_begun(today) &&
          !reservation->is_checkout_settled()) {
        schedule_cancellation(reservation);
      }
    }
    return;
  }

  for (Reservation *reservation : hotel_.get_reservations()) {
    if (reservation == nullptr || reservation->get_is_cancelled()) {
      continue;
    }
    if (reservation->get_check_in() != today || reservation->is_checked_in()) {
      continue;
    }

    const unsigned int checkin_minutes =
        static_cast<unsigned int>(reservation->get_hour_check_in()) * 60U +
        static_cast<unsigned int>(reservation->get_minute_check_in());
    const unsigned int now_minutes = hour * 60U + minute;
    if (now_minutes >= checkin_minutes + kNoShowGraceMinutes) {
      schedule_cancellation(reservation);
    }
  }
}

void Simulation::process_cancellations() {
  for (Reservation *reservation : pending_cancellations_) {
    if (reservation == nullptr || reservation->get_is_cancelled()) {
      continue;
    }
    const unsigned int room_nr = reservation->get_room().get_number();
    hotel_.cancel_reservation(reservation, actual_date());
    log_event(sim_event_category::rezerwacja,
              "Hotel anulowal rezerwacje pokoju nr " +
                  std::to_string(room_nr));
  }
  pending_cancellations_.clear();
}

void Simulation::tick_process_pending_cancellations() {
  process_cancellations();
}
