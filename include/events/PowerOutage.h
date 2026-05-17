//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_ZAD4_POWEROUTAGE_H
#define PROI_ZAD4_POWEROUTAGE_H

#include "Hotel.h"
#include "HotelEvent.h"

class PowerOutage : public HotelEvent {
public:
  PowerOutage(Hotel &h);
  ~PowerOutage() override;

  PowerOutage(const PowerOutage &other) = delete;
  PowerOutage &operator=(const PowerOutage &other) = delete;
};

#endif // PROI_ZAD4_POWEROUTAGE_H
