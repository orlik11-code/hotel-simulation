//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_HOTEL_SYSTEM_MANAGEMENT_RESERVATION_H
#define PROI_HOTEL_SYSTEM_MANAGEMENT_RESERVATION_H
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
  Reservation(const std::vector<Person> &p, unsigned int requested_room);
  const std::vector<Person> &get_people() const;
  unsigned int get_room_number() const;
  ~Reservation() = default;

private:
  unsigned int room_number;
  const std::vector<Person> people;
};

#endif // PROI_HOTEL_SYSTEM_MANAGEMENT_RESERVATION_H
