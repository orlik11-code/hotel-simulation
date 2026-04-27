//
// Created by orlik on 27.04.2026.
//

#include "../include/DoubleRoom.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

DoubleRoom::DoubleRoom(double area, unsigned int number, unsigned int floor,
                       unsigned int single_beds, unsigned int double_beds,
                       bool to_clean, bool to_repair)
    : Room(area, this->calc_price(), DoubleRoom::calc_workers_needed(), number,
           floor, single_beds, double_beds, to_clean, to_repair) {
  if (single_beds != 0 && double_beds != 0) {
    throw std::logic_error("Given parameters:\n" + std::to_string(single_beds) +
                           " single beds,\n" + std::to_string(double_beds) +
                           " double beds.\n" +
                           "Not correct for DoubleRoom where only proper "
                           "configurations are: (1 + 1), (2 + 0), (0 + 1)");
  } else if (double_beds > 1) {
    throw std::logic_error("Given parameters:\n" + std::to_string(double_beds) +
                           "double beds.\n" + "Not a DoubleRoom, " +
                           std::to_string(double_beds - 1) +
                           " double beds should be thrown away.");
  } else if (single_beds > 2) {
    throw std::logic_error("Given parameters:\n" + std::to_string(single_beds) +
                           "single beds.\n" + "Not a DoubleRoom, " +
                           std::to_string(single_beds - 2) +
                           " single beds should be thrown away.");
  }
}

const unsigned int DoubleRoom::calc_price() const {
  unsigned int base_price = 170;
  unsigned int floor_addon = 15 * get_floor();
  const unsigned int res = base_price + floor_addon;
  return res;
}

unsigned int DoubleRoom::calc_workers_needed() { return (const unsigned int)2; }

bool Room::check_suitability(
    std::vector<unsigned int> &people_sleeping_together) {

  unsigned int sum_of_people = std::accumulate(
      people_sleeping_together.begin(), people_sleeping_together.end(), 0);
  if (sum_of_people > 2 || sum_of_people == 0) {
    throw(std::out_of_range("Trying to put " + std::to_string(sum_of_people) +
                            " in DoubleRoom."));
  }
  unsigned int d_beds = this->get_double_beds();
  unsigned int p2 = std::ranges::count(people_sleeping_together, 2);

  return (p2 == 0 || d_beds > 0);
}