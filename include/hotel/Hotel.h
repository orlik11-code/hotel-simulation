#ifndef HOTEL_H
#define HOTEL_H

#include "booking/Reservation.h"
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
  // a co z rezerwacjami ? podepnij prosze rezerwacje tutaj
  std::vector<std::unique_ptr<Reservation>> reservations;
  // zrób kolekcje z rezerwacjami abym mógł podpiąć
  // std::vector<Reservation> reservations

  /* tutaj lepiej trzymac unique_ptr, rezerwacje beda tworzone
  w innej klasie, raczej w simulation, wiec narazimy sie na kopiowanie, po
  prostu zrob std::make_unique i uzyj add_reservation
  */

  bool power;
  bool evacuation;
  bool actual_failure = false;
  /** Start doby operacyjnej (np. poranny batch sprzątania). */
  unsigned int hotel_hour_start = 8;
  /** Koniec okna dispatch pokojówek (recepcja — tylko grafik staff). */
  unsigned int hotel_hour_end = 16;

  int balance = 0;

public:
  Hotel();

  [[nodiscard]] bool check_existance(unsigned int worker_uid) const;
  [[nodiscard]] bool get_actual_failure() const noexcept;
  [[nodiscard]] unsigned int get_hotel_hour_start() const noexcept;
  [[nodiscard]] unsigned int get_hotel_hour_end() const noexcept;
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
  /** Zwraca pracownika, który dostał zadanie, albo nullptr. */
  [[nodiscard]] Staff *assign_first_available_staff(
      StaffRole role, const Task &task, unsigned int hour_of_day,
      unsigned int weekday_index);

  void addRoom(std::unique_ptr<Room> room);
  void removeRoom(unsigned int roomNumber);
  void showReport() const;

  void set_power(bool power);
  void set_evacuation(bool evacuation);

  void add_reservation(std::unique_ptr<Reservation> reservation_up);

  int get_balance() const;
  void change_balance(int change);

  // anulowanie przy 2 mniej dniach płacą dziadygi :D
  void cancel_reservation(Reservation *reservation,
                          std::chrono::sys_days hotel_time);

  // cały bill trafia do salda hoteli
  void complete_checkout(Reservation *reservation);

  std::vector<Reservation *> get_reservations() const;

  // set flag na brudny pokoj (konkretny)
  bool mark_room_dirty(unsigned int room_number);
};

#endif