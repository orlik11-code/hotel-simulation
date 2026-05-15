//
// Created by orlik on 15.05.2026.
//

#include "../include/Reservation.h"

Reservation::Reservation(const std::vector<Person> &p,
                         unsigned int requested_room)
    : room_number(requested_room), people(p) {}

unsigned int Reservation::get_room_number() const { return room_number; }

const std::vector<Person> &Reservation::get_people() const { return people; }