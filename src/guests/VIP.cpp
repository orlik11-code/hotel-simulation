#include "../include/guests/VIP.hpp"

void VIP::reduce_satisfaction(int default_value) {
  satisfaction -= (default_value * 5);
  satisfaction = std::max(satisfaction, 0);
}