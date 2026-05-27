#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../include/booking/Bill.h"
#include "../include/booking/Reservation.h"
#include "../include/guests/Business.hpp"
#include "../include/rooms/TripleRoom.h"
#include "doctest.h"

TEST_CASE("Bill calculation logic with unique_ptr") {
  TripleRoom room(50.0, 101, 1, 3, 0, false, false);

  std::vector<std::unique_ptr<Guest>> guests;
  guests.push_back(std::make_unique<Business>("Jan", "Kowalski"));
  guests.push_back(std::make_unique<Business>("Anna", "Nowak"));

  std::chrono::year_month_day check_in{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(1)};
  std::chrono::year_month_day check_out{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(4)};

  SUBCASE("Bill with food included") {
    Reservation res(std::move(guests), &room, check_in, check_out, true, 50);
    Bill bill(res);

    // TripleRoom nightly: 220 + (20 * 1) = 240
    // (3 nights * 240) + (50 food * 2 people) = 720 + 100 = 820
    CHECK(bill.get_total_bill() == 820);
  }

  SUBCASE("Bill without food included") {
    std::vector<std::unique_ptr<Guest>> guests2;
    guests2.push_back(std::make_unique<Business>("Jan", "Kowalski"));
    guests2.push_back(std::make_unique<Business>("Anna", "Nowak"));
    Reservation res(std::move(guests2), &room, check_in, check_out, false, 50);
    Bill bill(res);

    // (3 nights * 240) + 0 = 720
    CHECK(bill.get_total_bill() == 720);
  }
}