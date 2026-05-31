//
// Created by orlik on 15.05.2026.
//

#include "booking/Reservation.h"

#include <chrono>
#include <stdexcept>

// dorobić budrzet hotelowy czy ty zrobisz? mysle ze to by było mega spoko
// zwłaszcz, że mamy juz logikę peniedzy praktycznie podpięte reservation
// powinno mieć flage resolved zeby wiedziec czy juz ta rezerwacja przeszła czy
// nie set_resolved i get_resolved

/* Last updates:

1. Check in and check out added as std::chrono::year_month_day.
2. Nights are being calculated from check in and check out and given as
std::chrono::days
3. bool check_resolved() checks if reservation has finished
4. bool check_beginned() checks if reservation has begun
5. Add field is_paid which shows if client has already paid for the reservation.
6. Payment may be proceeded by void proceed_payment() but only once, when client
has already paid, then exception will be thrown.

*/
Reservation::Reservation(const std::vector<std::unique_ptr<Guest>> &p,
                         const Room *requested_room,
                         std::chrono::year_month_day c_in,
                         std::chrono::year_month_day c_out, bool food_included,
                         unsigned int food_price)
    : check_in(c_in), check_out(c_out), room(requested_room), people(p),
      food_included(food_included), food_price(food_price) {
  if (room == nullptr) {
    throw std::invalid_argument(
        "Room must not be nullptr to make a reservation.");
  }
  if (c_in >= c_out) {
    throw std::out_of_range("Check out date must be after check in date.");
  }
  nights = std::chrono::sys_days{c_out} - std::chrono::sys_days{c_in};
}

bool Reservation::check_resolved(std::chrono::sys_days hotel_time) const {
  if (hotel_time == std::chrono::sys_days{check_out}) {
    return true;
  }
  return false;
}

bool Reservation::check_has_begun(std::chrono::sys_days hotel_time) const {
  if (hotel_time >= std::chrono::sys_days{check_in}) {
    return true;
  }
  return false;
}

void Reservation::proceed_payment() {
  if (is_paid) {
    throw std::logic_error("Client has already paid for the reservation. "
                           "Payment cannot be proceeded again!");
  }
  is_paid = true;
  return;
}

bool Reservation::get_is_paid() const { return this->is_paid; }

const Room &Reservation::get_room() const { return *room; }

const std::vector<std::unique_ptr<Guest>> &Reservation::get_people() const {
  return people;
}

std::chrono::days Reservation::get_nights() const { return nights; }

bool Reservation::get_food_included() const { return food_included; }

unsigned int Reservation::get_food_price() const { return food_price; };

bool Reservation::get_check_out_request() const { return check_out_request; }

void Reservation::request_check_out(std::chrono::sys_days hotel_time) {
  if (this->check_resolved(hotel_time)) {
    check_out_request = true;
    return;
  }
  auto dates_difference = hotel_time - std::chrono::sys_days{check_out};
  int detailed_error_info =
      std::chrono::duration_cast<std::chrono::days>(dates_difference).count();
  throw(std::logic_error(
      "Check out may be requested on the last day of stay. Today is " +
      std::to_string(detailed_error_info) + " days before that."));
}

std::chrono::sys_days Reservation::get_check_in() const { return check_in; }

std::chrono::sys_days Reservation::get_check_out() const { return check_out; }

void Reservation::cancel_by_guest(std::chrono::sys_days hotel_time) {
  if (cancelled) {
    return;
  }
  cancelled = true;

  if (hotel_time - std::chrono::sys_days{check_in} >= std::chrono::days{2}) {
    is_paid = true;
  } else {
    set_payment_no_effort();
  }
}

void Reservation::cancel_by_hotel(std::chrono::sys_days hotel_time) {
  if (cancelled) {
    return;
  }
  cancelled = true;
  is_paid = true;
  if (hotel_time - std::chrono::sys_days{check_in} < std::chrono::days{1}) {
    for (const auto &guest : people) {
      guest->reduce_satisfaction(1);
    }
  }
}

bool Reservation::get_is_cancelled() const { return cancelled; }

bool Reservation::is_checked_in() const { return checked_in; }

bool Reservation::is_checkout_settled() const { return checkout_settled; }

void Reservation::mark_checked_in() { checked_in = true; }

void Reservation::mark_checkout_settled() { checkout_settled = true; }

void Reservation::set_payment_no_effort() { is_paid = true; }