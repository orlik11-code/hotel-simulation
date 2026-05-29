#include "simulation/Simulation.hpp"

#include <stdexcept>
#include <string>

namespace {

constexpr unsigned int kFailureDestroyAfterMinutes = 40U;

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
  if (reservation == nullptr) {
    return;
  }
  pending_cancellations_.push_back(reservation);
}

void Simulation::process_cancellations() {
  for (Reservation *reservation : pending_cancellations_) {
    hotel_.cancel_reservation(reservation, actual_date());
  }
  pending_cancellations_.clear();
}

void Simulation::tick_process_pending_cancellations() {
  process_cancellations();
}
