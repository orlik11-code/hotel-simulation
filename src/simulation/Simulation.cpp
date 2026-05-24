#include "simulation/Simulation.hpp"

#include "booking/Reservation.h"
#include "staff/types.hpp"

#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

constexpr unsigned int kFailureDestroyAfterMinutes = 40U;
constexpr unsigned int kCheckOutHour = 8U;
constexpr unsigned int kCheckInHour = 11U;

/** Maps std::chrono weekday to simulation index (0 = Monday, 6 = Sunday). */
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

Simulation::Simulation(const SimConfig &new_sim_config)
    : hotel(), config(new_sim_config), current_tick(0U), is_running(true),
      time_with_fail(0U), event_generator(std::make_unique<EventGenerator>(
                              hotel, config.event_probability)) {}

Hotel &Simulation::get_hotel() { return hotel; }

void Simulation::log_event(const std::string &category,
                         const std::string &message) {
  event_log_[category].push_back(
      {message, static_cast<int>(current_tick)});
}

const SimEventLog &Simulation::get_event_log() const noexcept {
  return event_log_;
}

void Simulation::set_config(const SimConfig &sim_config) {
  config = sim_config;
  event_generator =
      std::make_unique<EventGenerator>(hotel, config.event_probability);
}

void Simulation::try_fix_failure() {
  if (hotel.has_pending_maintenance()) {
    time_with_fail += config.minutesPerTick;
    if (time_with_fail >= kFailureDestroyAfterMinutes) {
      throw std::runtime_error("Hotel has been destroyed");
    }
    return;
  }

  if (hotel.try_assign_maintenance(hour_of_day(), weekday())) {
    log_event(sim_event_category::awaria,
              "Przypisano konserwatora do naprawy awarii.");
    return;
  }

  time_with_fail += config.minutesPerTick;
  if (time_with_fail >= kFailureDestroyAfterMinutes) {
    throw std::runtime_error("Hotel has been destroyed");
  }
}

void Simulation::apply_guest_satisfaction_penalty(Reservation *reservation,
                                                  int penalty_points) {
  (void)reservation;
  (void)penalty_points;
  // TODO zadowolenie: reservation->penalize_guests(penalty_points) albo pętla
  // po get_people() → Guest::reduce_satisfaction (VIP/Business/Family).
  // Wymaga poprawnej polimorfii gości — patrz docs/KONRAD_RESERVATION_KROKI.md krok 4.
}

bool Simulation::should_run_daily_dirty(
    unsigned int hour_of_day, std::chrono::sys_days today) const {
  if (hour_of_day < hotel.get_hotel_hour_start()) {
    return false;
  }
  if (last_daily_dirty_date.has_value() && *last_daily_dirty_date == today) {
    return false;
  }
  return true;
}

void Simulation::process_daily_dirty_rooms(std::chrono::sys_days today) {
  unsigned int marked = 0U;

  for (Reservation *reservation : hotel.get_reservations()) {
    if (reservation == nullptr || reservation->get_is_cancelled()) {
      continue;
    }

    const bool active_stay =
        reservation->check_has_begun(today) &&
        !reservation->check_resolved(today);
    const bool checkout_today = reservation->get_check_out() == today;

    if (!active_stay && !checkout_today) {
      continue;
    }

    if (hotel.mark_room_dirty(reservation->get_room().get_number())) {
      ++marked;
    }
  }

  last_daily_dirty_date = today;
  std::ostringstream msg;
  msg << "Dzienny reset o " << hotel.get_hotel_hour_start()
      << ":00 — oznaczono " << marked << " pokoi jako brudne.";
  log_event(sim_event_category::sprzatanie, msg.str());
}

bool Simulation::has_pending_reception(Reservation *reservation) const {
  if (reservation == nullptr) {
    return false;
  }
  for (const PendingReceptionTask &pending : pending_reception) {
    if (pending.reservation == reservation) {
      return true;
    }
  }
  return false;
}

void Simulation::process_reception_completions() {
  auto it = pending_reception.begin();
  while (it != pending_reception.end()) {
    if (it->reservation == nullptr ||
        it->reservation->get_is_cancelled()) {
      it = pending_reception.erase(it);
      continue;
    }

    Staff *worker = hotel.find_by_uid(it->worker_uid);
    if (worker == nullptr) {
      it = pending_reception.erase(it);
      continue;
    }

    if (worker->get_is_busy()) {
      // TODO zadowolenie: gość czeka na zakończenie tasku recepcji — opcjonalnie
      // apply_guest_satisfaction_penalty(it->reservation, 1) co tick.
      ++it;
      continue;
    }

    if (worker->get_current_task().type != TaskType::CheckIn) {
      ++it;
      continue;
    }

    const unsigned int room_nr =
        it->reservation->get_room().get_number();
    if (it->is_checkout) {
      hotel.complete_checkout(it->reservation);
      it->reservation->mark_checkout_settled();
      log_event(sim_event_category::checkout,
                "Wymeldowano pokoj nr " + std::to_string(room_nr));
    } else {
      it->reservation->mark_checked_in();
      log_event(sim_event_category::check_in,
                "Zameldowano pokoj nr " + std::to_string(room_nr));
    }

    it = pending_reception.erase(it);
  }
}

void Simulation::check_checkout(unsigned int hour, unsigned int day) {
  if (hotel.get_actual_failure()) {
    return;
  }

  const std::chrono::sys_days today = actual_date();

  for (Reservation *reservation : hotel.get_reservations()) {
    if (reservation == nullptr || reservation->get_is_cancelled()) {
      continue;
    }

    const bool checkin_today = reservation->get_check_in() == today;
    const bool checkout_today = reservation->get_check_out() == today;

    if (hour == kCheckInHour && checkin_today && !reservation->is_checked_in() &&
        !has_pending_reception(reservation)) {
      const Task reception_task = Task::make_check_in();
      Staff *receptionist = hotel.assign_first_available_staff(
          StaffRole::Receptionist, reception_task, hour, day);
      if (receptionist != nullptr) {
        pending_reception.push_back(
            PendingReceptionTask{reservation, false, receptionist->get_uid()});
        log_event(sim_event_category::check_in,
                  "Przypisano recepcjonistke " +
                      std::to_string(receptionist->get_uid()) +
                      " do pokoju nr " +
                      std::to_string(reservation->get_room().get_number()));
      } else {
        // TODO zadowolenie: brak wolnej recepcji w dniu meldunku.
        // apply_guest_satisfaction_penalty(reservation, 1);
      }
      continue;
    }

    if (hour != kCheckOutHour) {
      continue;
    }

    if (!reservation->check_has_begun(today)) {
      continue;
    }

    if ((checkout_today || reservation->get_check_out_request()) &&
        !reservation->is_checkout_settled() &&
        !has_pending_reception(reservation)) {
      const Task reception_task = Task::make_check_in();
      Staff *receptionist = hotel.assign_first_available_staff(
          StaffRole::Receptionist, reception_task, hour, day);
      if (receptionist != nullptr) {
        pending_reception.push_back(
            PendingReceptionTask{reservation, true, receptionist->get_uid()});
        log_event(sim_event_category::checkout,
                  "Przypisano recepcjonistke " +
                      std::to_string(receptionist->get_uid()) +
                      " do wymeldowania pokoju nr " +
                      std::to_string(reservation->get_room().get_number()));
      } else {
        // TODO zadowolenie: brak recepcji przy wymeldowaniu.
        // apply_guest_satisfaction_penalty(reservation, 1);
      }
    }
  }

  // TODO zadowolenie (opcjonalnie): rezerwacje w pending_reception już dłużej niż N
  // ticków bez completion — dodatkowa kara; trzymać licznik ticków w PendingReceptionTask.
}

void Simulation::schedule_cancellation(Reservation *reservation) {
  if (reservation == nullptr) {
    return;
  }
  pending_cancellations.push_back(reservation);
}

void Simulation::process_cancellations() {
  for (Reservation *reservation : pending_cancellations) {
    hotel.cancel_reservation(reservation, actual_date());
  }
  pending_cancellations.clear();
}

void Simulation::tick() {
  current_tick++;
  hotel.tick(config.minutesPerTick);

  const bool had_failure = hotel.get_actual_failure();
  hotel.apply_completed_maintenance();
  if (had_failure && !hotel.get_actual_failure()) {
    time_with_fail = 0U;
    if (active_event_ != nullptr) {
      active_event_->clear();
      active_event_.reset();
    }
    log_event(sim_event_category::awaria, "Awaria naprawiona.");
  }

  process_reception_completions();

  const unsigned int hour = hour_of_day();
  const unsigned int day = weekday();

  if (hotel.get_actual_failure()) {
    try_fix_failure();
  } else if (std::unique_ptr<HotelEvent> event =
                 event_generator->decide_what_happens()) {
    active_event_ = std::move(event);
    active_event_->apply();
    log_event(sim_event_category::awaria,
              std::string(active_event_->description()) +
                  " (szansa " + std::to_string(config.event_probability) +
                  "%).");
    try_fix_failure();
  }

  process_cancellations();

  const std::chrono::sys_days today = actual_date();
  if (should_run_daily_dirty(hour, today)) {
    process_daily_dirty_rooms(today);
  }

  check_checkout(hour, day);

  hotel.apply_completed_housekeeping();
  if (!hotel.get_actual_failure()) {
    const HousekeepingStats hk_stats =
        hotel.collect_housekeeping_stats(hour, day);
    const unsigned int hk_assigned =
        hotel.dispatch_housekeeping(hour, day);
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
    msg << "Minal 1 tick (" << config.minutesPerTick
        << " min). Obecny tick: " << current_tick << " (godz. " << hour
        << ", dzien_tygodnia " << day << ")";
    log_event(sim_event_category::czas, msg.str());
  }
}

void Simulation::run() {
  log_event(sim_event_category::symulacja,
            "Prawdopodobienstwo zdarzenia: " +
                std::to_string(config.event_probability) + "%");

  if (config.max_ticks > 0U) {
    log_event(sim_event_category::symulacja,
              "Start — " + std::to_string(config.max_ticks) + " tickow.");
    while (is_running && current_tick < config.max_ticks) {
      tick();
    }
    is_running = false;
    log_event(sim_event_category::symulacja,
              "Koniec po " + std::to_string(current_tick) + " tickach.");
    return;
  }

  log_event(sim_event_category::symulacja,
            "Start — Enter = kolejny tick, q = koniec.");
  while (is_running) {
    std::cout << ">>> ";
    std::string cmd;
    if (!std::getline(std::cin, cmd)) {
      is_running = false;
      break;
    }
    if (cmd == "q" || cmd == "Q") {
      is_running = false;
      break;
    }
    tick();
  }
  log_event(sim_event_category::symulacja,
            "Koniec po " + std::to_string(current_tick) + " tickach.");
}

unsigned int Simulation::get_current_tick() const noexcept {
  return current_tick;
}

unsigned int Simulation::days_elapsed() const noexcept {
  if (current_tick == 0U) {
    return 0U;
  }
  return (current_tick - 1U) / config.ticsPerDay;
}

std::chrono::sys_days Simulation::actual_date() const noexcept {
  return get_actual_date(days_elapsed());
}

unsigned int Simulation::hour_of_day() const noexcept {
  if (current_tick == 0U) {
    return 0U;
  }
  const unsigned int slot_in_day = (current_tick - 1U) % config.ticsPerDay;
  const unsigned int minutes_from_midnight =
      slot_in_day * config.minutesPerTick;
  return (minutes_from_midnight / 60U) % 24U;
}

unsigned int Simulation::weekday() const noexcept {
  if (!config.date_start.ok()) {
    return days_elapsed() % 7U;
  }
  return weekday_index_from_sys_days(actual_date());
}

std::chrono::sys_days
Simulation::get_actual_date(unsigned int days_elapsed) const noexcept {
  return std::chrono::sys_days{config.date_start} +
         std::chrono::days{static_cast<int>(days_elapsed)};
}

unsigned int Simulation::minutes_per_tick() const noexcept {
  return config.minutesPerTick;
}
