#include "../include/guests/Business.hpp"

void Business::reduce_satisfaction(int default_value) {
  satisfaction -= (default_value * 3);
  satisfaction = std::max(satisfaction, 0);
}