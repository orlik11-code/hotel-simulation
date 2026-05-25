#pragma once

#include <random>

/** Wspólny generator losowy symulacji (taski, restauracja; EventGenerator — TODO). */
[[nodiscard]] std::mt19937 &simulation_rng();

/** Liczba z [min_inclusive, max_inclusive], jednakowe prawdopodobieństwo. */
[[nodiscard]] unsigned int random_unsigned(unsigned int min_inclusive,
                                           unsigned int max_inclusive);

/** Los 1..100; true gdy wynik <= percent (0 = false, >=100 = true). */
[[nodiscard]] bool random_chance(unsigned int percent);