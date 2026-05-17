//
// Created by orlik on 27.04.2026.
//

#ifndef PROI_ZAD4_DOUBLEROOM_H
#define PROI_ZAD4_DOUBLEROOM_H
#include "rooms/Room.h"

class DoubleRoom : public Room {
public:
  DoubleRoom(double area, unsigned int number, unsigned int floor,
             unsigned int single_beds, unsigned int double_beds, bool to_clean,
             bool to_repair);

  bool check_suitability(
      const std::vector<unsigned int> &people_sleeping_together) override;

  unsigned int calculateNightlyPrice() const override;
  unsigned int calculateStaffNeeded() const override;
};

#endif // PROI_ZAD4_DOUBLEROOM_H
