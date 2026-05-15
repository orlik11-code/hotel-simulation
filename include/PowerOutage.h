//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_ZAD4_POWEROUTAGE_H
#define PROI_ZAD4_POWEROUTAGE_H

#include "Hotel.h"

class PowerOutage {
public:
  PowerOutage(Hotel &h);
  ~PowerOutage();

  PowerOutage(const PowerOutage &other) = delete;
  PowerOutage &operator=(const PowerOutage &other) = delete;

private:
  Hotel &hotel;
  bool active;
};

#endif // PROI_ZAD4_POWEROUTAGE_H
