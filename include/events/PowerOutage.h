//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_ZAD4_POWEROUTAGE_H
#define PROI_ZAD4_POWEROUTAGE_H

#include "events/HotelEvent.h"
#include "hotel/Hotel.h"

class PowerOutage : public HotelEvent {
public:
  explicit PowerOutage(Hotel &h);
  ~PowerOutage() override;

  void apply() override;
  void clear() override;
  [[nodiscard]] const char *description() const override;

  PowerOutage(const PowerOutage &other) = delete;
  PowerOutage &operator=(const PowerOutage &other) = delete;
};

#endif // PROI_ZAD4_POWEROUTAGE_H
