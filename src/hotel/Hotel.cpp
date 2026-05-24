#include "hotel/Hotel.h"

#include "booking/Bill.h"
#include "booking/Reservation.h"
#include "rooms/Room.h"
#include "staff.hpp"
#include "staff/types.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

namespace {

[[nodiscard]] Weekday weekday_from_index(unsigned int weekday_index) {
  switch (weekday_index % 7U) {
  case 0U:
    return Weekday::monday;
  case 1U:
    return Weekday::tuesday;
  case 2U:
    return Weekday::wednesday;
  case 3U:
    return Weekday::thursday;
  case 4U:
    return Weekday::friday;
  case 5U:
    return Weekday::saturday;
  default:
    return Weekday::sunday;
  }
}

} // namespace

Hotel::Hotel() {}

bool Hotel::check_existance(unsigned int worker_uid) const {
  const auto found = std::ranges::find_if(
      workers, [worker_uid](const std::unique_ptr<Staff> &worker) {
        return worker->get_uid() == worker_uid;
      });
  return found != workers.end();
}

void Hotel::add_worker(std::unique_ptr<Staff> worker) {
  if (worker) {
    workers.push_back(std::move(worker));
  }
}

Staff *Hotel::find_by_uid(unsigned int uid) {
  const auto found =
      std::ranges::find_if(workers, [&](const std::unique_ptr<Staff> &worker) {
        return worker->get_uid() == uid;
      });

  if (found == workers.end()) {
    return nullptr;
  }
  return found->get();
}

std::vector<Staff *> Hotel::find_by_role(StaffRole role) {
  std::vector<Staff *> result;
  auto filtered_view =
      workers |
      std::views::filter([role](const std::unique_ptr<Staff> &worker) {
        return worker->role() == role;
      }) |
      std::views::transform(
          [](const std::unique_ptr<Staff> &worker) { return worker.get(); });
  std::ranges::copy(filtered_view, std::back_inserter(result));

  return result;
}

bool Hotel::assign_task(unsigned int uid, const Task &task,
                        unsigned int hour_of_day, unsigned int weekday_index) {
  Staff *correct_worker = find_by_uid(uid);
  if (correct_worker == nullptr) {
    return false;
  }
  const Weekday wd = weekday_from_index(weekday_index);
  if (!correct_worker->can_handle(task, wd, hour_of_day)) {
    return false;
  }
  correct_worker->do_work(task, wd, hour_of_day);
  return true;
}

Staff *Hotel::assign_first_available_staff(StaffRole role, const Task &task,
                                           unsigned int hour_of_day,
                                           unsigned int weekday_index) {
  const Weekday wd = weekday_from_index(weekday_index);
  auto available_view =
      workers | std::views::filter([&](const std::unique_ptr<Staff> &worker) {
        return worker->role() == role && !worker->is_tired() &&
               !worker->get_is_busy() &&
               worker->can_handle(task, wd, hour_of_day);
      });

  if (std::ranges::empty(available_view)) {
    return nullptr;
  }

  auto least_tired_it = std::ranges::min_element(
      available_view, std::less{}, [](const std::unique_ptr<Staff> &worker) {
        return worker->get_fatigue();
      });

  Staff *worker = least_tired_it->get();
  worker->do_work(task, wd, hour_of_day);
  return worker;
}

bool Hotel::assign_first_available(StaffRole role, const Task &task,
                                   unsigned int hour_of_day,
                                   unsigned int weekday_index) {
  return assign_first_available_staff(role, task, hour_of_day,
                                      weekday_index) != nullptr;
}

void Hotel::tick(unsigned int elapsed_minutes) {
  std::ranges::for_each(workers, [&](const std::unique_ptr<Staff> &worker) {
    worker->process_tick(elapsed_minutes);
  });
}

void Hotel::addRoom(std::unique_ptr<Room> room) {
  rooms.push_back(std::move(room));
}

void Hotel::removeRoom(unsigned int roomNumber) {
  auto it = std::remove_if(rooms.begin(), rooms.end(),
                           [roomNumber](const std::unique_ptr<Room> &r) {
                             return r->get_number() == roomNumber;
                           });
  rooms.erase(it, rooms.end());
}

void Hotel::showReport() const {
  for (const auto &room : rooms) {
    std::cout << "Pokoj nr: " << room->get_number()
              << " | Cena: " << room->calculateNightlyPrice()
              << " | Obsluga: " << room->calculateStaffNeeded() << std::endl;
  }
}

bool Hotel::get_actual_failure() const noexcept { return actual_failure; }

bool Hotel::set_actual_failure(bool value) {
  actual_failure = value;
  return actual_failure;
}

void Hotel::set_power(bool on) { power = on; }

void Hotel::set_evacuation(bool on) { evacuation = on; }

bool Hotel::get_power() const noexcept { return power; }

bool Hotel::get_evacuation() const noexcept { return evacuation; }

unsigned int Hotel::get_hotel_hour_start() const noexcept {
  return hotel_hour_start;
}

unsigned int Hotel::get_hotel_hour_end() const noexcept {
  return hotel_hour_end;
}

void Hotel::add_reservation(std::unique_ptr<Reservation> reservation_up) {
  this->reservations.push_back(std::move(reservation_up));
}

int Hotel::get_balance() const { return balance; }

void Hotel::change_balance(int change) { balance += change; }

void Hotel::cancel_reservation(Reservation *reservation,
                               const std::chrono::sys_days hotel_time) {
  if (reservation == nullptr || reservation->get_is_cancelled()) {
    return;
  }

  const bool before_stay = hotel_time < reservation->get_check_in();
  const bool during_stay = reservation->check_has_begun(hotel_time) &&
                           hotel_time < reservation->get_check_out();
  const std::chrono::days days_until_checkin =
      reservation->get_check_in() - hotel_time;
  const bool free_cancel =
      before_stay && days_until_checkin > std::chrono::days{2};

  reservation->cancel(hotel_time);

  if (!free_cancel && (before_stay || during_stay)) {
    const Bill bill(*reservation);
    change_balance(static_cast<int>(bill.get_total_bill()));
  }
}

void Hotel::complete_checkout(Reservation *reservation) {
  if (reservation == nullptr || reservation->get_is_cancelled()) {
    return;
  }
  const Bill bill(*reservation);
  change_balance(static_cast<int>(bill.get_total_bill()));
  if (!reservation->get_is_paid()) {
    reservation->proceed_payment();
  }
}

std::vector<Reservation *> Hotel::get_reservations() const {
  std::vector<Reservation *> res;

  auto view = reservations |
              std::views::transform([](const auto &ptr) { return ptr.get(); });

  std::ranges::copy(view, std::back_inserter(res));
  return res;
}

bool Hotel::mark_room_dirty(unsigned int room_number) {
  const auto found = std::ranges::find_if(
      rooms, [room_number](const std::unique_ptr<Room> &room) {
        return room && room->get_number() == room_number;
      });

  if (found == rooms.end()) {
    return false;
  }

  (*found)->make_dirty();
  return true;
}


bool Hotel::has_pending_housekeeping(const Room *room) const {
  if (room == nullptr) {
    return false;
  }
  for (const PendingHousekeeping &pending : pending_housekeeping) {
    if (pending.room == room || pending.room_number == room->get_number()) {
      return true;
    }
  }
  return false;
}

HousekeepingStats Hotel::collect_housekeeping_stats(
    unsigned int hour_of_day, unsigned int weekday_index) const {
  HousekeepingStats stats;
  const Weekday wd = weekday_from_index(weekday_index);
  const Task clean_probe = Task::make_clean_room();

  for (const std::unique_ptr<Room> &room : rooms) {
    if (room == nullptr || !room->is_to_clean()) {
      continue;
    }
    ++stats.dirty_rooms;
    if (has_pending_housekeeping(room.get())) {
      ++stats.already_pending;
    } else {
      ++stats.need_assignment;
    }
  }

  for (const std::unique_ptr<Staff> &worker : workers) {
    if (worker == nullptr || worker->role() != StaffRole::Housekeeper) {
      continue;
    }
    if (!worker->can_handle(clean_probe, wd, hour_of_day)) {
      continue;
    }
    ++stats.housekeepers_on_shift;
    if (!worker->get_is_busy() && !worker->is_tired()) {
      ++stats.housekeepers_free;
    }
  }

  return stats;
}

void Hotel::apply_completed_housekeeping() {
  auto it = pending_housekeeping.begin();
  while (it != pending_housekeeping.end()) {
    if (it->room == nullptr) {
      it = pending_housekeeping.erase(it);
      continue;
    }

    Staff *worker = find_by_uid(it->worker_uid);
    if (worker == nullptr) {
      it = pending_housekeeping.erase(it);
      continue;
    }

    if (worker->get_is_busy()) {
      ++it;
      continue;
    }

    if (worker->get_current_task().type != TaskType::CleanRoom) {
      ++it;
      continue;
    }

    it->room->clean();
    it = pending_housekeeping.erase(it);
  }
}


unsigned int Hotel::dispatch_housekeeping(unsigned int hour_of_day,
                                          unsigned int weekday_index) {
  if (get_actual_failure()) {
    return 0U;
  }
  if (hour_of_day < get_hotel_hour_start() ||
      hour_of_day >= get_hotel_hour_end()) {
    return 0U;
  }

  const HousekeepingStats stats =
      collect_housekeeping_stats(hour_of_day, weekday_index);
  const unsigned int to_assign =
      std::min(stats.need_assignment, stats.housekeepers_free);
  unsigned int assigned = 0U;

  for (unsigned int i = 0U; i < to_assign; ++i) {
    Room *target = nullptr;
    for (const std::unique_ptr<Room> &room_up : rooms) {
      if (room_up && room_up->is_to_clean() &&
          !has_pending_housekeeping(room_up.get())) {
        target = room_up.get();
        break;
      }
    }
    if (target == nullptr) {
      break;
    }

    const Task task = Task::make_clean_room();
    Staff *keeper = assign_first_available_staff(
        StaffRole::Housekeeper, task, hour_of_day, weekday_index);
    if (keeper == nullptr) {
      break;
    }

    pending_housekeeping.push_back(
        {target, keeper->get_uid(), target->get_number()});
    ++assigned;
  }

  return assigned;
}

std::vector<std::unique_ptr<Guest>> &Hotel::emplace_guest_list() {
  guest_lists.emplace_back();
  return guest_lists.back();
}

bool Hotel::has_pending_maintenance() const {
  return !pending_maintenance.empty();
}

void Hotel::apply_completed_maintenance() {
  auto it = pending_maintenance.begin();
  while (it != pending_maintenance.end()) {
    Staff *worker = find_by_uid(it->worker_uid);
    if (worker == nullptr) {
      it = pending_maintenance.erase(it);
      continue;
    }

    if (worker->get_is_busy()) {
      ++it;
      continue;
    }

    if (worker->get_current_task().type != TaskType::FixFailure) {
      ++it;
      continue;
    }

    actual_failure = false;
    it = pending_maintenance.erase(it);
  }
}

bool Hotel::try_assign_maintenance(unsigned int hour_of_day,
                                   unsigned int weekday_index) {
  if (!actual_failure || has_pending_maintenance()) {
    return false;
  }

  const Task task = Task::make_fix_failure();
  Staff *worker = assign_first_available_staff(
      StaffRole::MaintenanceWorker, task, hour_of_day, weekday_index);
  if (worker == nullptr) {
    return false;
  }

  pending_maintenance.push_back({worker->get_uid()});
  return true;
}