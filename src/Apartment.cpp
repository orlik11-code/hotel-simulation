//
// Created by orlik on 27.04.2026.
//

#include "../include/Apartment.h"

Apartment::Apartment(double area, unsigned int number, unsigned int floor,
                     unsigned int single_beds, unsigned int double_beds,
                     bool to_clean, bool to_repair)
    : Room(area, number, floor, single_beds, double_beds, to_clean, to_repair) {
}

unsigned int Apartment::calculateNightlyPrice() const {
  unsigned int base_price = 170;
  unsigned int beds_price = 10 * (get_single_beds() + get_double_beds());
  unsigned int floor_price = 20 * get_floor();

  unsigned int res = base_price + beds_price + floor_price;
  return res;
}

unsigned int Apartment::calculateStaffNeeded() const {
  unsigned int res = ((get_single_beds() + get_double_beds()) / 3) + 1;
  return res;
}
