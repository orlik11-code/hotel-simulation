#include "simulation/Simulation.hpp"

#include "booking/Reservation.h"
#include "simulation/simulation_rng.hpp"
#include "staff/types.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

Simulation::Simulation(const SimConfig &new_sim_config)
    : hotel_(), config_(new_sim_config), current_tick_(0U), is_running_(true),
      time_with_fail_(0U), event_generator_(std::make_unique<EventGenerator>(
                              hotel_, config_.event_probability)) {}

Hotel &Simulation::get_hotel() { return hotel_; }
// FLAG FLAG
void Simulation::set_live_console_output(bool enabled) noexcept {
  live_console_output_ = enabled;
}

void Simulation::log_event(const std::string &category,
                         const std::string &message) {
  event_log_[category].push_back(
      {message, static_cast<int>(current_tick_)});

  // FLAG FLAG
  if (!live_console_output_) {
    return;
  }

  std::cout << '[' << format_virtual_time() << "] [" << category << "] "
            << message << '\n';
  std::cout.flush();
}

const SimEventLog &Simulation::get_event_log() const noexcept {
  return event_log_;
}

void Simulation::set_config(const SimConfig &sim_config) {
  config_ = sim_config;
  event_generator_ =
      std::make_unique<EventGenerator>(hotel_, config_.event_probability);
}






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
                "Gosc nie przyszedl na " +
                    std::string(meal_type_label(slot)) + " (pokoj nr " +
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
    if (worker != nullptr && worker->get_is_busy()) {
      ++it;
      continue;
    }

    const unsigned int room_nr = it->reservation->get_room().get_number();

    if (it->phase == MealPhase::Cooking) {
      if (worker != nullptr &&
          worker->get_current_task().type != TaskType::CookMeal) {
        ++it;
        continue;
      }
      it->phase = MealPhase::Ready;
      it->worker_uid = 0U;
      log_event(sim_event_category::restauracja,
                "Posilek " + std::string(meal_type_label(it->meal_type)) +
                    " gotowy (pokoj nr " + std::to_string(room_nr) + ")");
      ++it;
      continue;
    }

    if (worker != nullptr &&
        worker->get_current_task().type != TaskType::ServeMeal) {
      ++it;
      continue;
    }

    meals_served_.push_back(
        MealServedRecord{it->reservation, it->meal_type, today});
    log_event(sim_event_category::restauracja,
              "Podano " + std::string(meal_type_label(it->meal_type)) +
                  " (pokoj nr " + std::to_string(room_nr) + ")");
    it = pending_meals_.erase(it);
  }
}

void Simulation::dispatch_meals(MealType slot, unsigned int hour,
                                unsigned int day) {
  if (hotel_.get_actual_failure() || hotel_.get_evacuation()) {
    return;
  }

  const std::chrono::sys_days today = actual_date();

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

    if (!has_pending_meal(reservation, slot)) {
      const Task cook_task = Task::make_cook_meal();
      Staff *chef = hotel_.assign_first_available_staff(StaffRole::Chef, cook_task,
                                                       hour, day);
      if (chef != nullptr) {
        pending_meals_.push_back(PendingMealTask{
            reservation, slot, MealPhase::Cooking, chef->get_uid()});
        log_event(sim_event_category::restauracja,
                  "Kucharz " + std::to_string(chef->get_uid()) + " gotuje " +
                      meal_type_label(slot) + " (pokoj nr " +
                      std::to_string(room_nr) + ")");
      }
      continue;
    }

    for (PendingMealTask &pending : pending_meals_) {
      if (pending.reservation != reservation || pending.meal_type != slot ||
          pending.phase != MealPhase::Ready || pending.worker_uid != 0U) {
        continue;
      }

      const Task serve_task = Task::make_serve_meal();
      Staff *waiter = hotel_.assign_first_available_staff(
          StaffRole::Waiter, serve_task, hour, day);
      if (waiter != nullptr) {
        pending.worker_uid = waiter->get_uid();
        log_event(sim_event_category::restauracja,
                  "Kelner " + std::to_string(waiter->get_uid()) + " podaje " +
                      meal_type_label(slot) + " (pokoj nr " +
                      std::to_string(room_nr) + ")");
      }
      break;
    }
  }
}

void Simulation::tick() {
  current_tick_++;
  hotel_.tick(config_.minutesPerTick);

  const bool had_failure = hotel_.get_actual_failure();
  hotel_.apply_completed_maintenance();
  if (had_failure && !hotel_.get_actual_failure()) {
    time_with_fail_ = 0U;
    if (active_event_ != nullptr) {
      active_event_->clear();
      active_event_.reset();
    }
    log_event(sim_event_category::awaria, "Awaria naprawiona.");
  }

  complete_pending_reception_tasks();

  const unsigned int hour = hour_of_day();
  const unsigned int day = weekday();

  if (hotel_.get_actual_failure()) {
    try_fix_failure();
  } else if (std::unique_ptr<HotelEvent> event =
                 event_generator_->decide_what_happens()) {
    active_event_ = std::move(event);
    active_event_->apply();
    log_event(sim_event_category::awaria,
              std::string(active_event_->description()) +
                  " (szansa " + std::to_string(config_.event_probability) +
                  "%).");
    try_fix_failure();
  }

  process_cancellations();

  const std::chrono::sys_days today = actual_date();
  if (should_run_daily_dirty(hour, today)) {
    process_daily_dirty_rooms(today);
  }

  schedule_check_in_and_check_out(hour, day);

  complete_pending_meal_tasks();
  if (!hotel_.get_actual_failure() && !hotel_.get_evacuation()) {
    if (const std::optional<MealType> slot = current_meal_slot(hour)) {
      ensure_meal_attendance_plans(*slot, today);
      dispatch_meals(*slot, hour, day);
    }
  }

  hotel_.apply_completed_housekeeping();
  if (!hotel_.get_actual_failure()) {
    const HousekeepingStats hk_stats =
        hotel_.collect_housekeeping_stats(hour, day);
    const unsigned int hk_assigned =
        hotel_.dispatch_housekeeping(hour, day);
    if (hk_stats.dirty_rooms > 0U || hk_assigned > 0U) {
      std::ostringstream msg;
      msg << "brudne: " << hk_stats.dirty_rooms << ", w kolejce: "
          << hk_stats.need_assignment
          << ", pokojowki wolne: " << hk_stats.housekeepers_free
          << ", przypisano: " << hk_assigned;
      log_event(sim_event_category::sprzatanie, msg.str());
    }
  }

  {
    std::ostringstream msg;
    msg << "Minal 1 tick (" << config_.minutesPerTick
        << " min). Obecny tick: " << current_tick_ << " (godz. " << hour
        << ", dzien_tygodnia " << day << ")";
    log_event(sim_event_category::czas, msg.str());
  }
}

void Simulation::run() {


  log_event(sim_event_category::symulacja,
            "Prawdopodobienstwo zdarzenia: " +
                std::to_string(config_.event_probability) + "%");

  if (config_.max_ticks > 0U) {
    log_event(sim_event_category::symulacja,
              "Start — " + std::to_string(config_.max_ticks) + " tickow.");
    while (is_running_ && current_tick_ < config_.max_ticks) {
      tick();
    }
    is_running_ = false;
    log_event(sim_event_category::symulacja,
              "Koniec po " + std::to_string(current_tick_) + " tickach.");

    return;
  }

  log_event(sim_event_category::symulacja,
            "Start — Enter = kolejny tick, q = koniec.");
  while (is_running_) {
    std::cout << ">>> ";
    std::string cmd;
    if (!std::getline(std::cin, cmd)) {
      is_running_ = false;
      break;
    }
    if (cmd == "q" || cmd == "Q") {
      is_running_ = false;
      break;
    }
    tick();
  }
  log_event(sim_event_category::symulacja,
            "Koniec po " + std::to_string(current_tick_) + " tickach.");
}









std::chrono::sys_days
Simulation::get_actual_date(unsigned int days_elapsed) const noexcept {
  return std::chrono::sys_days{config_.date_start} +
         std::chrono::days{static_cast<int>(days_elapsed)};
}

unsigned int Simulation::minutes_per_tick() const noexcept {
  return config_.minutesPerTick;
}