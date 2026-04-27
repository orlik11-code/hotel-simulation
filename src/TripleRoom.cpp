//
// Created by orlik on 27.04.2026.
//

#include "../include/TripleRoom.h"

#include <ranges>
#include <stdexcept>
#include <unordered_set>

TripleRoom::TripleRoom(double area, unsigned int number, unsigned int floor,
                       unsigned int single_beds, unsigned int double_beds,
                       bool to_clean, bool to_repair)
    : Room(area, this->calc_price(), TripleRoom::calc_workers_needed(), number,
           floor, single_beds, double_beds, to_clean, to_repair) {
  std::vector<std::pair<int, int>> correct_bed_configs{{3, 0}, {1, 1}, {0, 2}};
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