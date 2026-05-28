#pragma once

#include "simulation/SimConfig.hpp"

#include <string>

namespace sim_config_parse {

void apply_key(SimConfig &config, const std::string &key, const std::string &value);

} // namespace sim_config_parse
