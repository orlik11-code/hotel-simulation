//
// Created by orlik on 27.04.2026.
//

#ifndef PROI_ZAD4_APARTMENT_H
#define PROI_ZAD4_APARTMENT_H

#include "rooms/Room.h"

class Apartment : public Room {
public:
  Apartment(double area, unsigned int number, unsigned int floor,
            unsigned int single_beds, unsigned int double_beds, bool to_clean,
            bool to_repair);

  unsigned int calculateNightlyPrice() const override;
  unsigned int calculateStaffNeeded() const override;
};

#endif // PROI_ZAD4_APARTMENT_H
