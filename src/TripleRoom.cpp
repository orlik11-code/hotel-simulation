//
// Created by orlik on 27.04.2026.
//

#include "../include/TripleRoom.h"

#include <algorithm>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <unordered_set>

TripleRoom::TripleRoom(double area, unsigned int number, unsigned int floor,
                       unsigned int single_beds, unsigned int double_beds,
                       bool to_clean, bool to_repair)
    : Room(area, this->calc_price(), TripleRoom::calc_workers_needed(), number,
           floor, single_beds, double_beds, to_clean, to_repair) {
  std::vector<std::pair<unsigned int, unsigned int>> correct_bed_configs{
      {3, 0}, {1, 1}, {0, 2}};
  std::pair<unsigned int, unsigned int> proposed_config = {single_beds,
                                                           double_beds};
  if (std::ranges::find(correct_bed_configs, proposed_config) ==
      correct_bed_configs.end()) {
    throw std::logic_error(
        "Given bed config is not valid. Your (single, double): (" +
        std::to_string(single_beds) + "," + std::to_string(double_beds) +
        "). Possible only: (3, 0), (1, 1), (0, 2).");
  }
}

const unsigned int TripleRoom::calc_price() const {
  unsigned int base_price = 220;
  unsigned int floor_addon = 20 * get_floor();
  const unsigned int res = base_price + floor_addon;
  return res;
}

unsigned int TripleRoom::calc_workers_needed() { return (const unsigned int)2; }

bool Room::check_suitability(
    std::vector<unsigned int> &people_sleeping_together) {

  unsigned int sum_of_people = std::accumulate(
      people_sleeping_together.begin(), people_sleeping_together.end(), 0);
  if (sum_of_people > 3 || sum_of_people == 0) {
    throw(std::out_of_range("Trying to put " + std::to_string(sum_of_people) +
                            " in TripleRoom."));
  }
  
  unsigned int s_beds = this->get_single_beds();
  unsigned int d_beds = this->get_double_beds();
  unsigned int p1 = std::ranges::count(people_sleeping_together, 1);
  unsigned int p2 = std::ranges::count(people_sleeping_together, 2);

  if (p1 + p2 != (int)people_sleeping_together.size()) {
    throw(std::logic_error("Only 1 or 2 people can sleep together."));
  }

  d_beds -= p2;
  p1 -= std::min(std::max((unsigned int)0, d_beds), p1);
  s_beds -= p1;

  return (std::min(s_beds, d_beds) >= (unsigned int)0);
}