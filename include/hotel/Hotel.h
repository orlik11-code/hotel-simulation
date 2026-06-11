#ifndef HOTEL_H
#define HOTEL_H

#include "booking/Reservation.h"
#include "guests/Guest.hpp"
#include "rooms/Room.h"
#include "staff.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

struct PendingHousekeeping {
  Room *room = nullptr;
  unsigned int worker_uid = 0U;
  unsigned int room_number = 0U;
};

struct PendingMaintenance {
  unsigned int worker_uid = 0U;
};

struct HousekeepingStats {
  unsigned int dirty_rooms = 0;
  unsigned int already_pending = 0;
  unsigned int need_assignment = 0;
  unsigned int housekeepers_on_shift = 0;
  unsigned int housekeepers_free = 0;
};

class Hotel {
private:
  std::vector<std::unique_ptr<Staff>> workers;
  /** Guests from config — one vector per reservation (unique_ptr: Guest is abstract). */
  std::vector<std::vector<std::unique_ptr<Guest>>> guest_lists;
  std::vector<std::unique_ptr<Room>> rooms;
  std::vector<std::unique_ptr<Reservation>> reservations;

  /* Prefer unique_ptr: reservations are created elsewhere (e.g. Simulation);
     use add_reservation with std::make_unique to avoid copies. */

  bool power;
  bool evacuation;
  bool actual_failure = false;
  /** Start of the operational day (e.g. morning housekeeping batch). */
  unsigned int hotel_hour_start = 8;
  /** End of housekeeping dispatch window (reception uses staff schedules only). */
  unsigned int hotel_hour_end = 16;

  int balance = 0;

  std::vector<PendingHousekeeping> pending_housekeeping;
  std::vector<PendingMaintenance> pending_maintenance;

  [[nodiscard]] bool has_pending_housekeeping(const Room *room) const;

public:
  Hotel();

  [[nodiscard]] bool check_existance(unsigned int worker_uid) const;
  [[nodiscard]] bool get_actual_failure() const noexcept;
  [[nodiscard]] unsigned int get_hotel_hour_start() const noexcept;
  [[nodiscard]] unsigned int get_hotel_hour_end() const noexcept;
  bool set_actual_failure(bool value);
  void add_worker(std::unique_ptr<Staff> worker);
  void tick(unsigned int elapsed_minutes, unsigned int hour_of_day,
            unsigned int weekday_index);

  Staff *find_by_uid(unsigned int uid);
  std::vector<Staff *> find_by_role(StaffRole role);
  bool assign_task(unsigned int uid, const Task &task, unsigned int hour_of_day,
                   unsigned int weekday_index);
  [[nodiscard]] bool assign_first_available(StaffRole role, const Task &task,
                                            unsigned int hour_of_day,
                                            unsigned int weekday_index);
  /** Returns the worker who received the task, or nullptr. */
  [[nodiscard]] Staff *assign_first_available_staff(StaffRole role,
                                                    const Task &task,
                                                    unsigned int hour_of_day,
                                                    unsigned int weekday_index);

  void addRoom(std::unique_ptr<Room> room);
  void removeRoom(unsigned int roomNumber);
  void showReport() const;

  void set_power(bool power);
  void set_evacuation(bool evacuation);
  [[nodiscard]] bool get_power() const noexcept;
  [[nodiscard]] bool get_evacuation() const noexcept;

  void add_reservation(std::unique_ptr<Reservation> reservation_up);

  int get_balance() const;
  void change_balance(int change);

  /** Cancellation within 2 days of check-in may incur guest penalties. */
  void cancel_reservation(Reservation *reservation,
                          std::chrono::sys_days hotel_time);

  /** Adds the full bill amount to hotel balance. */
  void complete_checkout(Reservation *reservation);

  std::vector<Reservation *> get_reservations() const;

  /** Marks a specific room as dirty (to_clean). */
  bool mark_room_dirty(unsigned int room_number);

  [[nodiscard]] HousekeepingStats
  collect_housekeeping_stats(unsigned int hour_of_day,
                             unsigned int weekday_index) const;

  /** Completes cleaning: when housekeeper finished CleanRoom → room.clean(). */
  void apply_completed_housekeeping();
  [[nodiscard]] unsigned int dispatch_housekeeping(unsigned int hour_of_day,
                                                   unsigned int weekday_index);

  [[nodiscard]] bool has_pending_maintenance() const;
  /** True if any maintenance worker is on shift and can accept FixFailure. */
  [[nodiscard]] bool has_maintenance_on_shift(unsigned int hour_of_day,
                                              unsigned int weekday_index) const;
  /** Completes repair: when worker finished FixFailure → actual_failure = false. */
  void apply_completed_maintenance();
  /** Assigns one maintenance worker when failure is active and none pending. */
  [[nodiscard]] bool try_assign_maintenance(unsigned int hour_of_day,
                                            unsigned int weekday_index);

  std::vector<std::unique_ptr<Guest>> &emplace_guest_list();

  void
  remove_reservation(std::vector<std::unique_ptr<Reservation>>::iterator it,
                     std::chrono::sys_days today);
};

#endif