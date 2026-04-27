//
// Created by orlik on 27.04.2026.
//

#include "../include/SingleRoom.h"

#include <stdexcept>

SingleRoom::SingleRoom(double area, unsigned int number, unsigned int floor,
                       unsigned int single_beds, unsigned int double_beds,
                       bool to_clean, bool to_repair)
    : Room(area, this->calc_price(), SingleRoom::calc_workers_needed(), number,
           floor, single_beds, double_beds, to_clean, to_repair) {
  if (single_beds + double_beds > 1) {
    throw(std::logic_error(
        std::to_string(single_beds + double_beds) +
        " beds in SingleRoom. Should be dedicated for more people?"));
  } // assumed that sometimes SingleRoom may contain one but double bed
}

const unsigned int SingleRoom::calc_price() const {
  unsigned int base_price = 100;
  unsigned int floor_addon = 10 * get_floor();
  const unsigned int res = base_price + floor_addon;
  return res;
}

unsigned int SingleRoom::calc_workers_needed() { return (const unsigned int)1; }