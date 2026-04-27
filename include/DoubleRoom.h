//
// Created by orlik on 27.04.2026.
//

#ifndef PROI_ZAD4_DOUBLEROOM_H
#define PROI_ZAD4_DOUBLEROOM_H
#include "Room.h"

class DoubleRoom : public Room {
public:
  DoubleRoom(double area, unsigned int number, unsigned int floor,
             unsigned int single_beds, unsigned int double_beds, bool to_clean,
             bool to_repair);

private:
  const unsigned int calc_price() const;
  static unsigned int calc_workers_needed();
};

#endif // PROI_ZAD4_DOUBLEROOM_H
