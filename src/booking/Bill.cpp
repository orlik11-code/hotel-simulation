//
// Created by orlik on 15.05.2026.
//

#include "booking/Bill.h"
Bill::Bill(const Reservation &reservation) : reservation(&reservation) {
  const Room &room_to_pay = this->reservation->get_room();
  unsigned int nightly_bill = room_to_pay.calculateNightlyPrice();
  std::chrono::days nights = reservation.get_nights();
  auto nights_ui = static_cast<unsigned int>(nights.count());

  unsigned int people =
      static_cast<unsigned int>(reservation.get_people().size());
  unsigned int food_price = reservation.get_food_price();

  auto bill_food =
      static_cast<unsigned int>(reservation.get_food_included()) * food_price;

  this->total_bill = nights_ui * nightly_bill + bill_food * people;
}

unsigned int Bill::get_total_bill() const { return this->total_bill; }