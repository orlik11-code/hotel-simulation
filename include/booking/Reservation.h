//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_HOTEL_SYSTEM_MANAGEMENT_RESERVATION_H
#define PROI_HOTEL_SYSTEM_MANAGEMENT_RESERVATION_H
#include "../rooms/Room.h"
#include <chrono>
#include <string>
#include <vector>

struct Person {
  std::string name;
  std::string surname;
  std::string address;
  std::string phone;
  std::string email;
};

class Reservation {
public:
  Reservation(const std::vector<Person> &p, const Room *r,
              std::chrono::year_month_day check_in,
              std::chrono::year_month_day check_out, bool food_included,
              unsigned int food_price);
  const std::vector<Person> &get_people() const;
  const Room &get_room() const;
  std::chrono::days get_nights() const;
  std::chrono::sys_days get_check_in() const;
  std::chrono::sys_days get_check_out() const;
  bool check_resolved(std::chrono::sys_days hotel_time) const;
  bool check_has_begun(std::chrono::sys_days hotel_time) const;
  void proceed_payment();
  bool get_is_paid() const;
  bool get_food_included() const;
  unsigned int get_food_price() const;
  ~Reservation() = default;

private:
  std::chrono::sys_days check_in;
  std::chrono::sys_days check_out;
  const Room *room;
  const std::vector<Person> people;
  std::chrono::days nights;
  bool is_paid;
  bool food_included;
  unsigned int food_price;
};

#endif // PROI_HOTEL_SYSTEM_MANAGEMENT_RESERVATION_H
