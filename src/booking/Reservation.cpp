//
// Created by orlik on 15.05.2026.
//

#include "booking/Reservation.h"

#include <stdexcept>

// rezerwacja bez godziny i czasu, proszę dodaj chociaż dzień albo fajnie jakbyś dodał też konkretną godzine,
// póki co dodałem w klasie hotel.hpp - hotel_hour (godzina zameldowania i wymeldowania) mysle ze spoko ale do przegadania
// dorobić budrzet hotelowy czy ty zrobisz? mysle ze to by było mega spoko zwłaszcz, że mamy juz logikę peniedzy praktycznie podpięte
// reservation powinno mieć flage resolved zeby wiedziec czy juz ta rezerwacja przeszła czy nie set_resolved i get_resolved
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