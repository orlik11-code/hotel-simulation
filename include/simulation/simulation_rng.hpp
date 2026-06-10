#pragma once

#include <random>

/** Shared pseudo-random generator for simulation (tasks, meals, events). */
[[nodiscard]] std::mt19937 &simulation_rng();

/** Uniform random integer in [min_inclusive, max_inclusive]. */
[[nodiscard]] unsigned int random_unsigned(unsigned int min_inclusive,
                                           unsigned int max_inclusive);

/** Roll 1..100; true when result <= percent (0 = false, >=100 = true). */
[[nodiscard]] bool random_chance(unsigned int percent);