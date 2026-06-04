#include "simulation/Simulation.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "config/file_parse.hpp"
#include "logger/Logger.hpp"
#include "simulation/HotelReport.h"
#include "simulation/SimEventLog.hpp"

Simulation::Simulation(const SimConfig &new_sim_config)
    : hotel_(), config_(new_sim_config), current_tick_(0U), is_running_(true),
      time_with_fail_(0U),
      event_generator_(std::make_unique<EventGenerator>(
          hotel_, config_.event_probability)) {}

Hotel &Simulation::get_hotel() { return hotel_; }

void Simulation::set_live_console_output(bool enabled) noexcept {
  live_console_output_ = enabled;
}

bool Simulation::should_print_to_console(const std::string &category) const {
  if (!live_console_output_) {
    return false;
  }
  if (category != sim_event_category::czas) {
    return true;
  }
  const unsigned int interval = config_.log_every_n_ticks;
  if (interval <= 1U) {
    return true;
  }
  return current_tick_ % interval == 0U;
}

void Simulation::log_event(const std::string &category,
                           const std::string &message) {
  event_log_[category].push_back(
      {message, static_cast<int>(current_tick_)});

  if (!should_print_to_console(category)) {
    return;
  }

  std::cout << '[' << format_virtual_time() << "] [" << category << "] "
            << message << " (balance=" << hotel_.get_balance()
            << ", weekday=" << weekday() << ")\n";
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

void Simulation::tick_advance_hotel_clock() {
  ++current_tick_;
  hotel_.tick(config_.minutesPerTick, hour_of_day(), weekday());
}

void Simulation::tick_log_time_step(unsigned int hour, unsigned int weekday) {
  (void)hour;
  (void)weekday;
  const unsigned int interval =
      config_.log_every_n_ticks > 0U ? config_.log_every_n_ticks : 1U;
  std::ostringstream msg;
  if (interval <= 1U) {
    msg << "1 tick elapsed (" << config_.minutesPerTick
        << " min). Current tick: " << current_tick_;
  } else if (current_tick_ % interval == 0U) {
    const unsigned int elapsed_minutes = interval * config_.minutesPerTick;
    msg << interval << " ticks elapsed (" << elapsed_minutes
        << " min). Current tick: " << current_tick_;
  } else {
    msg << "Tick " << current_tick_;
  }
  log_event(sim_event_category::czas, msg.str());
}

void Simulation::tick() {
  tick_advance_hotel_clock();
  tick_resolve_maintenance();
  tick_process_reception_queue();

  const unsigned int hour = hour_of_day();
  const unsigned int day = weekday();
  const unsigned int minute = minutes_of_hour();

  tick_process_failures_and_random_events();
  tick_evaluate_hotel_cancellations(hour, minute);
  tick_process_pending_cancellations();
  tick_mark_daily_dirty_rooms();
  tick_schedule_reception_tasks(hour, minute, day);
  tick_process_meals(hour, day);
  tick_dispatch_housekeeping(hour, day);
  tick_log_time_step(hour, day);
  refresh_hotel_report();

}

void Simulation::run() {
  refresh_hotel_report();

  log_event(sim_event_category::symulacja,
            "Event probability: " +
                file_parse::format_probability_tenths(config_.event_probability));

  const unsigned int target_ticks =
      (config_.max_ticks > 0U)
          ? config_.max_ticks
          : (1440U + config_.minutesPerTick - 1U) / config_.minutesPerTick;
  log_event(sim_event_category::symulacja,
            "Auto-run started — " + std::to_string(target_ticks) + " ticks.");

  while (is_running_ && current_tick_ < target_ticks) {
    tick();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  is_running_ = false;
  log_event(sim_event_category::symulacja,
            "Finished after " + std::to_string(current_tick_) + " ticks.");

  publish_hotel_report();
  if (!config_.log_file.empty() && current_tick_ > 0U) {
    export_log_file();
    log_event(sim_event_category::symulacja,
              "Log exported to file: " + config_.log_file);
  }
}

void Simulation::export_log_file() const {
  const unsigned int file_interval =
      config_.log_file_every_n_ticks > 0U ? config_.log_file_every_n_ticks
                                            : config_.log_every_n_ticks;
  Logger logger(config_.log_file, *this, static_cast<int>(file_interval));
  logger.prepare_log(1, static_cast<int>(current_tick_),
                     sim_event_category::all_categories());
}

HotelReport Simulation::build_hotel_report() const {
  HotelReport report;
  report.balance = hotel_.get_balance();

  unsigned long long satisfaction_sum = 0U;
  unsigned int guest_count = 0U;

  for (Reservation *reservation : hotel_.get_reservations()) {
    if (reservation == nullptr || !reservation->is_checked_in()) {
      continue;
    }
    for (const std::unique_ptr<Guest> &guest : reservation->get_people()) {
      if (guest == nullptr) {
        continue;
      }
      const int satisfaction = guest->get_satisfaction();
      satisfaction_sum +=
          static_cast<unsigned long long>(satisfaction > 0 ? satisfaction : 0);
      ++guest_count;
    }
  }

  report.guest_count = guest_count;
  if (guest_count == 0U) {
    report.average_satisfaction = 0U;
    report.rating_label = "poor";
    return report;
  }

  report.average_satisfaction =
      static_cast<unsigned int>(satisfaction_sum / guest_count);

  if (report.average_satisfaction >= 80U) {
    report.rating_label = "excellent";
  } else if (report.average_satisfaction >= 60U) {
    report.rating_label = "good";
  } else if (report.average_satisfaction >= 40U) {
    report.rating_label = "average";
  } else {
    report.rating_label = "poor";
  }

  return report;
}

void Simulation::refresh_hotel_report() { hotel_report_ = build_hotel_report(); }

const HotelReport &Simulation::get_hotel_report() const noexcept {
  return hotel_report_;
}

void Simulation::publish_hotel_report() const {
  const HotelReport &report = get_hotel_report();
  std::cout << "REPORT: balance=" << report.balance
            << " guests=" << report.guest_count
            << " avg_satisfaction=" << report.average_satisfaction
            << " rating=" << report.rating_label << '\n';
}