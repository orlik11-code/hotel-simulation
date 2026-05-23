#include "../include/guests/Family.hpp"

void Family::reduce_satisfaction(int default_value) {
  satisfaction -= (default_value * 1);
  satisfaction = std::max(satisfaction, 0);
}