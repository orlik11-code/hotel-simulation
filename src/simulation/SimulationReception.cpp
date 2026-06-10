#include "simulation/Simulation.hpp"

#include "staff/types.hpp"

#include <string>

void Simulation::apply_guest_satisfaction_penalty(Reservation *reservation,
                                                  int penalty_points) {
  if (reservation == nullptr || reservation->get_is_cancelled()) {
    return;
  }
  reservation->penalize_waiting_guests(penalty_points);
}

bool Simulation::has_pending_reception(Reservation *reservation) const {
  if (reservation == nullptr) {
    return false;
  }
  for (const PendingReceptionTask &pending : pending_reception_) {
    if (pending.reservation == reservation) {
      return true;
    }
  }
  return false;
}

void Simulation::complete_pending_reception_tasks() {
  auto it = pending_reception_.begin();
  while (it != pending_reception_.end()) {
    if (it->reservation == nullptr || it->reservation->get_is_cancelled()) {
      it = pending_reception_.erase(it);
      continue;
    }

    Staff *worker = hotel_.find_by_uid(it->worker_uid);
    if (worker == nullptr) {
      it = pending_reception_.erase(it);
      continue;
    }

    if (worker->get_is_busy()) {
      apply_guest_satisfaction_penalty(it->reservation, 1);
      ++it;
      continue;
    }

    const TaskType expected_task =
        it->is_checkout ? TaskType::CheckOut : TaskType::CheckIn;
    if (worker->get_current_task().type != expected_task) {
      apply_guest_satisfaction_penalty(it->reservation, 1);
      ++it;
      continue;
    }

    const unsigned int room_nr = it->reservation->get_room().get_number();
    if (it->is_checkout) {
      hotel_.complete_checkout(it->reservation);
      it->reservation->mark_checkout_settled();
      log_event(sim_event_category::checkout,
                "Checked out room " + std::to_string(room_nr));
    } else {
      it->reservation->mark_checked_in();
      log_event(sim_event_category::check_in,
                "Checked in room " + std::to_string(room_nr));
      if (it->reservation->get_room().is_to_clean()) {
        apply_guest_satisfaction_penalty(it->reservation, 2);
        log_event(sim_event_category::housekeeping,
                  "Room " + std::to_string(room_nr) +
                      " was not cleaned before check-in");
      }
    }

    it = pending_reception_.erase(it);
  }
}

void Simulation::tick_process_reception_queue() {
  complete_pending_reception_tasks();
}

void Simulation::process_guest_checkout_requests(
    const std::chrono::sys_days today, unsigned int hour,
    unsigned int minute) {
  for (Reservation *reservation : hotel_.get_reservations()) {
    if (reservation == nullptr || reservation->get_is_cancelled()) {
      continue;
    }
    if (!reservation->is_checked_in() || reservation->is_checkout_settled()) {
      continue;
    }
    if (reservation->get_check_out() != today ||
        reservation->get_check_out_request()) {
      continue;
    }

    const unsigned int checkout_hour =
        static_cast<unsigned int>(reservation->get_hour_check_out());
    const unsigned int checkout_minute =
        static_cast<unsigned int>(reservation->get_minute_check_out());
    if (hour > checkout_hour ||
        (hour == checkout_hour && minute >= checkout_minute)) {
      reservation->request_check_out(today);
      log_event(sim_event_category::checkout,
                "Guest requested checkout for room " +
                    std::to_string(reservation->get_room().get_number()));
    }
  }
}

void Simulation::schedule_check_in_and_check_out(unsigned int hour,
                                                 unsigned int minute,
                                                 unsigned int weekday) {
  if (hotel_.get_evacuation()) {
    return;
  }

  const std::chrono::sys_days today = actual_date();
  process_guest_checkout_requests(today, hour, minute);

  for (Reservation *reservation : hotel_.get_reservations()) {
    if (reservation == nullptr || reservation->get_is_cancelled()) {
      continue;
    }

    const bool checkin_today = reservation->get_check_in() == today;
    const unsigned int checkin_hour =
        static_cast<unsigned int>(reservation->get_hour_check_in());
    const unsigned int checkin_minute =
        static_cast<unsigned int>(reservation->get_minute_check_in());
    const bool checkin_due =
        checkin_today &&
        (hour > checkin_hour ||
         (hour == checkin_hour && minute >= checkin_minute));

    const unsigned int checkout_hour =
        static_cast<unsigned int>(reservation->get_hour_check_out());
    const unsigned int checkout_minute =
        static_cast<unsigned int>(reservation->get_minute_check_out());
    const bool checkout_due =
        reservation->get_check_out_request() &&
        (hour > checkout_hour ||
         (hour == checkout_hour && minute >= checkout_minute));

    if (checkin_due && !reservation->is_checked_in() &&
        !has_pending_reception(reservation)) {
      const Task reception_task = Task::make_check_in();
      Staff *receptionist = hotel_.assign_first_available_staff(
          StaffRole::Receptionist, reception_task, hour, weekday);
      if (receptionist != nullptr) {
        pending_reception_.push_back(PendingReceptionTask{
            reservation, false, receptionist->get_uid()});
        log_event(sim_event_category::check_in,
                  "Receptionist " + std::to_string(receptionist->get_uid()) +
                      " assigned to room " +
                      std::to_string(reservation->get_room().get_number()));
      } else {
        apply_guest_satisfaction_penalty(reservation, 2);
      }
      continue;
    }

    if (!reservation->check_has_begun(today)) {
      continue;
    }

    if (checkout_due && !reservation->is_checkout_settled() &&
        !has_pending_reception(reservation)) {
      const Task reception_task = Task::make_check_out();
      Staff *receptionist = hotel_.assign_first_available_staff(
          StaffRole::Receptionist, reception_task, hour, weekday);
      if (receptionist != nullptr) {
        pending_reception_.push_back(PendingReceptionTask{
            reservation, true, receptionist->get_uid()});
        log_event(sim_event_category::checkout,
                  "Receptionist " + std::to_string(receptionist->get_uid()) +
                      " assigned to checkout room " +
                      std::to_string(reservation->get_room().get_number()));
      } else {
        apply_guest_satisfaction_penalty(reservation, 1);
      }
    }
  }
}

void Simulation::tick_schedule_reception_tasks(unsigned int hour,
                                               unsigned int minute,
                                               unsigned int weekday) {
  schedule_check_in_and_check_out(hour, minute, weekday);
}
