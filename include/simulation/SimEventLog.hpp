#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

/** Wiadomość + liczba (domyślnie tick symulacji). */
using SimEventRecord = std::pair<std::string, int>;

/** map<string, vector<pair<string,int>>> — odczyt: Konrad (Logger), zapis: Simulation. */
using SimEventLog = std::map<std::string, std::vector<SimEventRecord>>;

namespace sim_event_category {

inline constexpr const char *czas = "czas";
inline constexpr const char *sprzatanie = "sprzatanie";
inline constexpr const char *check_in = "check_in";
inline constexpr const char *checkout = "checkout";
inline constexpr const char *awaria = "awaria";
inline constexpr const char *symulacja = "symulacja";
inline constexpr const char *restauracja = "restauracja";
inline constexpr const char *rezerwacja = "rezerwacja";

} // namespace sim_event_category
