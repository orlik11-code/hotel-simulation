//
// Created by orlik on 15.05.2026.
//

#include "booking/Reservation.h"

#include <stdexcept>

Reservation::Reservation(const std::vector<Person> &p,
                         const Room *requested_room, unsigned int nights)
    : room(requested_room), people(p), nights(nights) {
  if (room == nullptr) {
    throw std::invalid_argument(
        "Room must not be nullptr to make a reservation.");
  }
}

const Room &Reservation::get_room() const { return *room; }

const std::vector<Person> &Reservation::get_people() const { return people; }

unsigned int Reservation::get_nights() const { return nights; }