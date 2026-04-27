//
// Created by orlik on 27.04.2026.
//

#ifndef PROI_ZAD4_SINGLEROOM_H
#define PROI_ZAD4_SINGLEROOM_H
#include "Room.h"

class SingleRoom : public Room {
public:
  SingleRoom(double area, unsigned int number, unsigned int floor,
             unsigned int single_beds, unsigned int double_beds, bool to_clean,
             bool to_repair);

private:
  unsigned int calc_price();
  unsigned int calc_workers_needed();
};

#endif // PROI_ZAD4_SINGLEROOM_H
