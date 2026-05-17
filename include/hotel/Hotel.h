#ifndef HOTEL_H
#define HOTEL_H

#include "rooms/Room.h"
#include "staff.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

class Hotel {
private:
  std::vector<std::unique_ptr<Staff>> workers;
  std::vector<std::unique_ptr<Room>> rooms;
  bool power;
  bool evacuation;
  bool actual_failure = false;

public:
  Hotel();

  [[nodiscard]] bool check_existance(unsigned int worker_uid) const;
  [[nodiscard]] bool get_actual_failure() const noexcept;
  bool set_actual_failure(bool value);
  void add_worker(std::unique_ptr<Staff> worker);
  void tick(unsigned int elapsed_minutes);

  Staff *find_by_uid(unsigned int uid);
  std::vector<Staff *> find_by_role(StaffRole role);
  bool assign_task(unsigned int uid, const Task &task, unsigned int hour_of_day,
                   unsigned int weekday_index);
  [[nodiscard]] bool assign_first_available(StaffRole role, const Task &task,
                                            unsigned int hour_of_day,
                                            unsigned int weekday_index);

  void addRoom(std::unique_ptr<Room> room);
  void removeRoom(unsigned int roomNumber);
  void showReport() const;

  void set_power(bool power);
  void set_evacuation(bool evacuation);
};

#endif