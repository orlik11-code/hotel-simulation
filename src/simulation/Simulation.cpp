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