//
// Created by orlik on 27.04.2026.
//

#include "../include/SingleRoom.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

SingleRoom::SingleRoom(double area, unsigned int number, unsigned int floor,
                       unsigned int single_beds, unsigned int double_beds,
                       bool to_clean, bool to_repair)
    : Room(area, number, floor, single_beds, double_beds, to_clean, to_repair) {
  if (single_beds + double_beds > 1) {
    throw(std::logic_error(
        std::to_string(single_beds + double_beds) +
        " beds in SingleRoom. Should be dedicated for more people?"));
  } // assumed that sometimes SingleRoom may contain one but double bed
}

unsigned int SingleRoom::calculateNightlyPrice() const {
  unsigned int base_price = 100;
  unsigned int floor_addon = 10 * get_floor();
  unsigned int res = base_price + floor_addon;
  return res;
}

unsigned int SingleRoom::calculateStaffNeeded() const {
  return (unsigned int)1;
}

bool SingleRoom::check_suitability(
    const std::vector<unsigned int> &people_sleeping_together) {

  unsigned int sum_of_people = std::accumulate(
      people_sleeping_together.begin(), people_sleeping_together.end(), 0);
  if (sum_of_people != 1) {
    throw(std::out_of_range("Trying to put " + std::to_string(sum_of_people) +
                            " in SingleRoom."));
  }
  return true;
}