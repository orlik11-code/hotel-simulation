//
// Created by orlik on 15.05.2026.
//

#include "../include/Bill.h"
Bill::Bill(Reservation &reservation) : reservation(&reservation) {
  const Room &room_to_pay = this->reservation->get_room();
  unsigned int nightly_bill = room_to_pay.calculateNightlyPrice();
  unsigned int nights = reservation.get_nights();

  this->total_bill = nights * nightly_bill;
}

unsigned int Bill::get_total_bill() const { return this->total_bill; }