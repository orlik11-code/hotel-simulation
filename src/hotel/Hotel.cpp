#include "hotel/Hotel.h"
#include "staff.hpp"

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

bool Hotel::assign_first_available(StaffRole role, const Task &task,
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
    return false;
  }

  auto least_tired_it = std::ranges::min_element(
      available_view, std::less{}, [](const std::unique_ptr<Staff> &worker) {
        return worker->get_fatigue();
      });

  (*least_tired_it)->do_work(task, wd, hour_of_day);

  return true;
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

void Hotel::set_power(bool on) { power = on; }

void Hotel::set_evacuation(bool on) { evacuation = on; }