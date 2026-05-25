#include "simulation/simulation_rng.hpp"

namespace {

std::mt19937 &engine() {
  static std::random_device seed_source;
  static std::mt19937 gen(seed_source());
  return gen;
}

} // namespace

std::mt19937 &simulation_rng() { return engine(); }

unsigned int random_unsigned(unsigned int min_inclusive,
                             unsigned int max_inclusive) {
  if (min_inclusive > max_inclusive) {
    return min_inclusive;
  }
  std::uniform_int_distribution<unsigned int> dist(min_inclusive, max_inclusive);
  return dist(engine());
}

bool random_chance(unsigned int percent) {
  if (percent == 0U) {
    return false;
  }
  if (percent >= 100U) {
    return true;
  }
  return random_unsigned(1U, 100U) <= percent;
}