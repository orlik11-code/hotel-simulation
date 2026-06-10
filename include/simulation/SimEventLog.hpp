#pragma once

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

/** Message + tick number (default: simulation tick). */
using SimEventRecord = std::pair<std::string, int>;

/** map<string, vector<pair<string,int>>> — read: Logger, write: Simulation. */
using SimEventLog = std::map<std::string, std::vector<SimEventRecord>>;

namespace sim_event_category {

inline constexpr const char *time = "time";
inline constexpr const char *housekeeping = "housekeeping";
inline constexpr const char *check_in = "check_in";
inline constexpr const char *checkout = "checkout";
inline constexpr const char *failure = "failure";
inline constexpr const char *simulation = "simulation";
inline constexpr const char *restaurant = "restaurant";
inline constexpr const char *reservation = "reservation";

[[nodiscard]] inline std::set<std::string> all_categories() {
  return {time,        housekeeping, check_in, checkout, failure,
          simulation,  restaurant,   reservation};
}

} // namespace sim_event_category
