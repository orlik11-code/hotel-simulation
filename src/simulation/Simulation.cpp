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
// FLAG FLAG
void Simulation::set_live_console_output(bool enabled) noexcept {
  live_console_output_ = enabled;
}

void Simulation::log_event(const std::string &category,
                         const std::string &message) {
  event_log_[category].push_back(
      {message, static_cast<int>(current_tick)});

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
  // TODO zadowolenie: reservation->penalize_guests(penalty_points) (API Konrada).
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

namespace {

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
  for (const MealServiceWindow &window : config.meal_windows) {
    if (hour >= window.start_hour && hour < window.end_hour) {
      return window.type;
    }
  }
  return std::nullopt;
}

const MealServiceWindow *
Simulation::find_meal_window(MealType meal_type) const noexcept {
  for (const MealServiceWindow &window : config.meal_windows) {
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

  for (Reservation *reservation : hotel.get_reservations()) {
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
    plan.will_attend = random_chance(config.meal_attendance_percent);

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

void Simulation::process_meal_completions() {
  const std::chrono::sys_days today = actual_date();
  auto it = pending_meals_.begin();
  while (it != pending_meals_.end()) {
    if (it->reservation == nullptr || it->reservation->get_is_cancelled()) {
      it = pending_meals_.erase(it);
      continue;
    }

    Staff *worker =
        it->worker_uid == 0U ? nullptr : hotel.find_by_uid(it->worker_uid);
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
  if (hotel.get_actual_failure() || hotel.get_evacuation()) {
    return;
  }

  const std::chrono::sys_days today = actual_date();

  for (Reservation *reservation : hotel.get_reservations()) {
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
      Staff *chef = hotel.assign_first_available_staff(StaffRole::Chef, cook_task,
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
      Staff *waiter = hotel.assign_first_available_staff(
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

  process_meal_completions();
  if (!hotel.get_actual_failure() && !hotel.get_evacuation()) {
    if (const std::optional<MealType> slot = current_meal_slot(hour)) {
      ensure_meal_attendance_plans(*slot, today);
      dispatch_meals(*slot, hour, day);
    }
  }

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

unsigned int Simulation::minutes_from_midnight() const noexcept {
  if (current_tick == 0U) {
    return 0U;
  }
  const unsigned int slot_in_day = (current_tick - 1U) % config.ticsPerDay;
  return slot_in_day * config.minutesPerTick;
}

std::string Simulation::format_virtual_time() const {
  const unsigned int total_minutes = minutes_from_midnight();
  const unsigned int hour = (total_minutes / 60U) % 24U;
  const unsigned int minute = total_minutes % 60U;

  std::ostringstream out;
  if (config.date_start.ok()) {
    const std::chrono::year_month_day ymd{actual_date()};
    out << std::setfill('0') << static_cast<int>(ymd.year()) << '-'
        << std::setw(2) << static_cast<unsigned>(ymd.month()) << '-'
        << std::setw(2) << static_cast<unsigned>(ymd.day());
  } else {
    out << "dzien-" << days_elapsed();
  }
  out << ' ' << std::setw(2) << hour << ':' << std::setw(2) << minute;
  return out.str();
}

unsigned int Simulation::hour_of_day() const noexcept {
  return (minutes_from_midnight() / 60U) % 24U;
}

unsigned int Simulation::minutes_of_hour() const noexcept {
  return minutes_from_midnight() % 60U;
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