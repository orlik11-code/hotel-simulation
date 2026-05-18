#include "simulation/Simulation.hpp"

#include "booking/Reservation.h"
#include "staff/types.hpp"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr unsigned int kFailureDestroyAfterMinutes = 40U;

/** Maps std::chrono weekday to simulation index (0 = Monday, 6 = Sunday). */
[[nodiscard]] unsigned int weekday_index_from_sys_days(std::chrono::sys_days day) {
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
      time_with_fail(0U),
      event_generator(std::make_unique<EventGenerator>(
          hotel, config.event_probability)) {}

Hotel &Simulation::get_hotel() { return hotel; }

void Simulation::set_config(const SimConfig &sim_config) {
  config = sim_config;
  event_generator = std::make_unique<EventGenerator>(hotel,
                                                     config.event_probability);
}

void Simulation::try_fix_failure() {
  const Task fix_failure{TaskType::FixFailure};
  const bool assigned = hotel.assign_first_available(
      StaffRole::MaintenanceWorker, fix_failure, hour_of_day(), weekday());
  if (assigned) {
    time_with_fail = 0U;
    hotel.set_actual_failure(false);
    return;
  }
  time_with_fail += config.minutesPerTick;
  if (time_with_fail >= kFailureDestroyAfterMinutes) {
    throw std::runtime_error("Hotel has been destroyed");
  }
}

// void Simulation::check_checkout(unsigned int hour, unsigned int day)
// {
//   (void)hour;
//   (void)day;
//   // TODO: odkomentować, gdy będą:
//   //   - Hotel::get_reservations() zwracające kolekcję Reservation
//   //   - Reservation::get_checkin() / get_checkout() zwracające std::chrono::sys_days
//   //   - (opcjonalnie) Hotel::get_hotel_hour_end()
//   // Recepcjoniści obsługują meldowanie/wymeldowanie w godzinach pracy hotelu.
//   // Zadanie recepcjonisty (Task::make_check_in()) trwa losowo 1–10 minut.
//   if (hour >= hotel.get_hotel_hour() && hour < /* hotel.get_hotel_hour_end() */ 16U)
//   {
//     for (const Reservation& reservation : hotel.get_reservations())
//     {
//       const bool is_checkin_today  = reservation.get_checkin()  == actual_date();
//       const bool is_checkout_today = reservation.get_checkout() == actual_date();
//       if (!is_checkin_today && !is_checkout_today && reservation.is_resoved())
//       {
//         continue;
//       }
//       if (is_checkin_today)
//       {
//       const Task reception_task = Task::make_check_in();

//       const bool assigned = hotel.assign_first_available(
//           StaffRole::Receptionist, reception_task, hour, day);
//       if (!assigned)
//       {
//   //       // Brak wolnego recepcjonisty — kara/awaria.
//   //       // Uwaga: time_with_fail jest współdzielone z try_fix_failure();
//   //       // docelowo lepiej trzymać osobne pole (np. time_with_no_receptionist).
//         time_with_fail += config.minutesPerTick / 10U;
//         if (time_with_fail >= kFailureDestroyAfterMinutes)
//         {
//           throw std::runtime_error("Hotel has been destroyed");
//         }
//       }
//       else {
//       reservation.set_resolved();
//       }
//       if (is_checkout_today)
//       {
//        hotel.add_balance(reservation.get_total_bill());
//       }
//     }
//   }

// }

void Simulation::tick() {
  current_tick++;
  hotel.tick(config.minutesPerTick);

  const unsigned int hour = hour_of_day();
  const unsigned int day = weekday();

  if (hotel.get_actual_failure())
  {
    try_fix_failure();
  }
  else if (const std::unique_ptr<HotelEvent> event =
                 event_generator->decide_what_happens())
  {
    std::cout << "[Zdarzenie] Wystapilo zdarzenie hotelowe (szansa "
              << config.event_probability << "%).\n";
    (void)event;
    hotel.set_actual_failure(true);
    try_fix_failure();
  }

  check_checkout(hour, day);

  std::cout << "[Czas] Minal 1 tick (" << config.minutesPerTick
            << " min). Obecny tick: " << current_tick << " (godz. " << hour
            << ", dzien_tygodnia " << day << ")\n";
}

void Simulation::run() {
  std::cout << "[Symulacja] Prawdopodobienstwo zdarzenia: "
            << config.event_probability << "%\n";

  if (config.max_ticks > 0U) {
    std::cout << "[Symulacja] Start — " << config.max_ticks << " tickow.\n";
    while (is_running && current_tick < config.max_ticks) {
      tick();
    }
    is_running = false;
    std::cout << "[Symulacja] Koniec po " << current_tick << " tickach.\n";
    return;
  }

  std::cout << "[Symulacja] Start — Enter = kolejny tick, q = koniec.\n";
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
  std::cout << "[Symulacja] Koniec po " << current_tick << " tickach.\n";
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

std::chrono::sys_days Simulation::get_actual_date(unsigned int days_elapsed) const noexcept {
  return std::chrono::sys_days{config.date_start} +
         std::chrono::days{static_cast<int>(days_elapsed)};
}

unsigned int Simulation::minutes_per_tick() const noexcept {
  return config.minutesPerTick;
}
