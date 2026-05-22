//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_HOTEL_SYSTEM_MANAGEMENT_BILL_H
#define PROI_HOTEL_SYSTEM_MANAGEMENT_BILL_H
#include "../rooms/Room.h"
#include "booking/Reservation.h"

class Bill {
public:
  Bill(const Reservation &reservation);
  unsigned int get_total_bill() const;
  ~Bill() = default;

private:
  const Reservation *reservation;
  unsigned int total_bill;
};

#endif // PROI_HOTEL_SYSTEM_MANAGEMENT_BILL_H
