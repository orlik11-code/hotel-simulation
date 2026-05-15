//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_HOTEL_SYSTEM_MANAGEMENT_RESERVATION_H
#define PROI_HOTEL_SYSTEM_MANAGEMENT_RESERVATION_H
#include "Room.h"
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
  Reservation(const std::vector<Person> &p, const Room *r, unsigned int nights);
  const std::vector<Person> &get_people() const;
  const Room &get_room() const;
  unsigned int get_nights() const;
  ~Reservation() = default;

private:
  const Room *room;
  const std::vector<Person> people;
  unsigned int nights;
};

#endif // PROI_HOTEL_SYSTEM_MANAGEMENT_RESERVATION_H
