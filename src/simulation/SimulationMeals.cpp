#include "simulation/Simulation.hpp"

#include "simulation/simulation_rng.hpp"
#include "staff/types.hpp"

#include <algorithm>
#include <sstream>
#include <string>

namespace {

constexpr unsigned int kChefBatchCapacity = 10U;

[[nodiscard]] const char *meal_type_label(MealType type) {
  return type == MealType::breakfast ? "breakfast" : "dinner";
}

[[nodiscard]] bool reservation_eligible_for_meal(Reservation *reservation,
                                                 std::chrono::sys_days today) {
  return reservation != nullptr && !reservation->get_is_cancelled() &&
         reservation->get_food_included() &&
         reservation->check_has_begun(today) &&
         !reservation->check_resolved(today);
}

} // namespace

std::optional<MealType> Simulation::current_meal_slot(unsigned int hour) const {
  for (const MealServiceWindow &window : config_.meal_windows) {
    if (hour >= window.start_hour && hour < window.end_hour) {
      return window.type;
    }
  }
  return std::nullopt;
}

const MealServiceWindow *
Simulation::find_meal_window(MealType meal_type) const noexcept {
  for (const MealServiceWindow &window : config_.meal_windows) {
    if (window.type == meal_type) {
      return &window;
    }
  }
  return nullptr;
}

bool Simulation::has_meal_plan(Reservation *reservation, MealType meal_type,
                               std::chrono::sys_days day) const {
  for (const MealAttendancePlan &plan : meal_attendance_plans_) {
    if (plan.reservation == reservation && plan.meal_type == meal_type &&
        plan.day == day) {
      return true;
    }
  }
  return false;
}

bool Simulation::already_served(Reservation *reservation, MealType meal_type,
                                std::chrono::sys_days day) const {
  for (const MealServedRecord &record : meals_served_) {
    if (record.reservation == reservation && record.meal_type == meal_type &&
        record.day == day) {
      return true;
    }
  }
  return false;
}

bool Simulation::has_no_show(Reservation *reservation, MealType meal_type,
                             std::chrono::sys_days day) const {
  for (const MealNoShowRecord &record : meal_no_shows_) {
    if (record.reservation == reservation && record.meal_type == meal_type &&
        record.day == day) {
      return true;
    }
  }
  return false;
}

bool Simulation::has_pending_meal(Reservation *reservation,
                                  MealType meal_type) const {
  for (const PendingMealTask &pending : pending_meals_) {
    if (pending.reservation == reservation && pending.meal_type == meal_type) {
      return true;
    }
  }
  return false;
}

const MealAttendancePlan *
Simulation::find_meal_plan(Reservation *reservation, MealType meal_type,
                           std::chrono::sys_days day) const {
  for (const MealAttendancePlan &plan : meal_attendance_plans_) {
    if (plan.reservation == reservation && plan.meal_type == meal_type &&
        plan.day == day) {
      return &plan;
    }
  }
  return nullptr;
}

void Simulation::ensure_meal_attendance_plans(MealType slot,
                                            std::chrono::sys_days today) {
  const MealServiceWindow *window = find_meal_window(slot);
  if (window == nullptr || window->start_hour >= window->end_hour) {
    return;
  }

  for (Reservation *reservation : hotel_.get_reservations()) {
    if (!reservation_eligible_for_meal(reservation, today)) {
      continue;
    }
    if (has_meal_plan(reservation, slot, today) ||
        already_served(reservation, slot, today) ||
        has_no_show(reservation, slot, today)) {
      continue;
    }

    MealAttendancePlan plan;
    plan.reservation = reservation;
    plan.meal_type = slot;
    plan.day = today;
    plan.will_attend = random_chance(config_.meal_attendance_percent);

    if (plan.will_attend) {
      plan.planned_arrival_hour =
          random_unsigned(window->start_hour, window->end_hour - 1U);
      meal_attendance_plans_.push_back(plan);
    } else {
      meal_no_shows_.push_back(MealNoShowRecord{reservation, slot, today});
      log_event(sim_event_category::restauracja,
                "Guest no-show for " +
                    std::string(meal_type_label(slot)) + " (room " +
                    std::to_string(reservation->get_room().get_number()) + ")");
    }
  }
}

void Simulation::complete_pending_meal_tasks() {
  const std::chrono::sys_days today = actual_date();
  auto it = pending_meals_.begin();
  while (it != pending_meals_.end()) {
    if (it->reservation == nullptr || it->reservation->get_is_cancelled()) {
      it = pending_meals_.erase(it);
      continue;
    }

    Staff *worker =
        it->worker_uid == 0U ? nullptr : hotel_.find_by_uid(it->worker_uid);

    if (it->phase == MealPhase::Ready && it->worker_uid == 0U) {
      apply_guest_satisfaction_penalty(it->reservation, 1);
      ++it;
      continue;
    }

    if (worker != nullptr && worker->get_is_busy()) {
      apply_guest_satisfaction_penalty(it->reservation, 1);
      ++it;
      continue;
    }

    const unsigned int room_nr = it->reservation->get_room().get_number();

    if (it->phase == MealPhase::Cooking) {
      if (worker != nullptr &&
          worker->get_current_task().type != TaskType::CookMeal) {
        apply_guest_satisfaction_penalty(it->reservation, 1);
        ++it;
        continue;
      }
      it->phase = MealPhase::Ready;
      it->worker_uid = 0U;
      log_event(sim_event_category::restauracja,
                std::string(meal_type_label(it->meal_type)) +
                    " ready (room " + std::to_string(room_nr) + ")");
      ++it;
      continue;
    }

    if (worker != nullptr &&
        worker->get_current_task().type != TaskType::ServeMeal) {
      apply_guest_satisfaction_penalty(it->reservation, 1);
      ++it;
      continue;
    }

    meals_served_.push_back(
        MealServedRecord{it->reservation, it->meal_type, today});
    log_event(sim_event_category::restauracja,
              "Served " + std::string(meal_type_label(it->meal_type)) +
                  " (room " + std::to_string(room_nr) + ")");
    it = pending_meals_.erase(it);
  }
}

void Simulation::dispatch_meals(MealType slot, unsigned int hour,
                                unsigned int weekday) {
  if (hotel_.get_actual_failure() || hotel_.get_evacuation()) {
    return;
  }

  const std::chrono::sys_days today = actual_date();
  const unsigned int minute = minutes_of_hour();
  const bool is_hour_slot_start = minute < config_.minutesPerTick;

  std::vector<Reservation *> to_cook;
  if (is_hour_slot_start) {
    for (Reservation *reservation : hotel_.get_reservations()) {
      if (!reservation_eligible_for_meal(reservation, today)) {
        continue;
      }
      if (already_served(reservation, slot, today) ||
          has_no_show(reservation, slot, today) ||
          has_pending_meal(reservation, slot)) {
        continue;
      }

      const MealAttendancePlan *plan =
          find_meal_plan(reservation, slot, today);
      if (plan == nullptr || !plan->will_attend ||
          plan->planned_arrival_hour != hour) {
        continue;
      }
      to_cook.push_back(reservation);
    }
  }

  for (size_t batch_start = 0U; batch_start < to_cook.size();
       batch_start += kChefBatchCapacity) {
    const size_t batch_end =
        std::min(batch_start + static_cast<size_t>(kChefBatchCapacity),
                 to_cook.size());
    std::vector<Reservation *> batch;
    batch.reserve(batch_end - batch_start);
    for (size_t i = batch_start; i < batch_end; ++i) {
      batch.push_back(to_cook[i]);
    }

    const Task cook_task = Task::make_cook_meal();
    Staff *chef = hotel_.assign_first_available_staff(
        StaffRole::Chef, cook_task, hour, weekday);
    if (chef != nullptr) {
      std::ostringstream rooms;
      for (Reservation *reservation : batch) {
        pending_meals_.push_back(PendingMealTask{
            reservation, slot, MealPhase::Cooking, chef->get_uid()});
        if (rooms.tellp() > 0) {
          rooms << ", ";
        }
        rooms << reservation->get_room().get_number();
      }
      log_event(sim_event_category::restauracja,
                "Chef " + std::to_string(chef->get_uid()) + " cooking " +
                    meal_type_label(slot) + " for rooms: " + rooms.str());
      continue;
    }

    for (Reservation *reservation : batch) {
      meal_no_shows_.push_back(MealNoShowRecord{reservation, slot, today});
      apply_guest_satisfaction_penalty(reservation, 1);
    }
    log_event(sim_event_category::restauracja,
              "No available chef for " + std::string(meal_type_label(slot)) +
                  " (" + std::to_string(batch.size()) + " rooms)");
  }

  for (Reservation *reservation : hotel_.get_reservations()) {
    if (!reservation_eligible_for_meal(reservation, today)) {
      continue;
    }
    if (already_served(reservation, slot, today) ||
        has_no_show(reservation, slot, today)) {
      continue;
    }

    const MealAttendancePlan *plan = find_meal_plan(reservation, slot, today);
    if (plan == nullptr || !plan->will_attend ||
        plan->planned_arrival_hour != hour) {
      continue;
    }

    const unsigned int room_nr = reservation->get_room().get_number();

    for (PendingMealTask &pending : pending_meals_) {
      if (pending.reservation != reservation || pending.meal_type != slot ||
          pending.phase != MealPhase::Ready || pending.worker_uid != 0U) {
        continue;
      }

      const Task serve_task = Task::make_serve_meal();
      Staff *waiter = hotel_.assign_first_available_staff(
          StaffRole::Waiter, serve_task, hour, weekday);
      if (waiter != nullptr) {
        pending.worker_uid = waiter->get_uid();
        log_event(sim_event_category::restauracja,
                  "Waiter " + std::to_string(waiter->get_uid()) + " serving " +
                      meal_type_label(slot) + " (room " +
                      std::to_string(room_nr) + ")");
      } else {
        apply_guest_satisfaction_penalty(reservation, 1);
        log_event(sim_event_category::restauracja,
                  "No available waiter for " +
                      std::string(meal_type_label(slot)) + " (room " +
                      std::to_string(room_nr) + ")");
      }
      break;
    }
  }
}

void Simulation::tick_process_meals(unsigned int hour, unsigned int weekday) {
  complete_pending_meal_tasks();
  if (hotel_.get_actual_failure() || hotel_.get_evacuation()) {
    return;
  }

  const std::optional<MealType> slot = current_meal_slot(hour);
  if (!slot.has_value()) {
    return;
  }

  ensure_meal_attendance_plans(*slot, actual_date());
  dispatch_meals(*slot, hour, weekday);
}
