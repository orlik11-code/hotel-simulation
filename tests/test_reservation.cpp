#include "../include/booking/Reservation.h"
#include "../include/guests/Business.hpp"
#include "../include/guests/Family.hpp"
#include "../include/guests/VIP.hpp"
#include "../include/rooms/DoubleRoom.h"
#include "../include/rooms/SingleRoom.h"
#include "doctest.h"

TEST_CASE("Reservation creation and basic properties") {
  SingleRoom room(20.0, 101, 1, 1, 0, false, false);

  std::vector<std::unique_ptr<Guest>> guests;
  guests.push_back(std::make_unique<Business>("John", "Doe"));
  guests.push_back(std::make_unique<Business>("Jane", "Smith"));

  std::chrono::year_month_day check_in{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(1)};
  std::chrono::year_month_day check_out{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(4)};

  Reservation res(std::move(guests), &room, check_in, check_out, true, 50);

  CHECK(res.get_people().size() == 2);
  CHECK(res.get_room().get_number() == 101);
  CHECK(res.get_food_included() == true);
  CHECK(res.get_food_price() == 50);
}

TEST_CASE("Reservation nights calculation") {
  SingleRoom room(20.0, 102, 1, 1, 0, false, false);

  std::vector<std::unique_ptr<Guest>> guests;
  guests.push_back(std::make_unique<VIP>("Alice", "Johnson"));

  std::chrono::year_month_day check_in{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(10)};
  std::chrono::year_month_day check_out{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(15)};

  Reservation res(std::move(guests), &room, check_in, check_out, false, 0);

  CHECK(res.get_nights() == std::chrono::days(5));
}

TEST_CASE("Reservation payment state") {
  DoubleRoom room(30.0, 103, 2, 2, 0, false, false);
  std::vector<std::unique_ptr<Guest>> guests;
  guests.push_back(std::make_unique<Family>("Tom", "Brown"));

  std::chrono::year_month_day check_in{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(1)};
  std::chrono::year_month_day check_out{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(3)};

  Reservation res(std::move(guests), &room, check_in, check_out, true, 40);

  CHECK(res.get_is_paid() == false);
  res.proceed_payment();
  CHECK(res.get_is_paid() == true);
}

TEST_CASE("Reservation check-in and check-out logic") {
  SingleRoom room(20.0, 104, 1, 1, 0, false, false);

  std::vector<std::unique_ptr<Guest>> guests;
  guests.push_back(std::make_unique<Business>("Mike", "Davis"));

  std::chrono::year_month_day check_in{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(5)};
  std::chrono::year_month_day check_out{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(7)};

  Reservation res(std::move(guests), &room, check_in, check_out, false, 0);

  auto check_in_sys = std::chrono::sys_days(check_in);
  auto check_out_sys = std::chrono::sys_days(check_out);
  auto before_check_in = check_in_sys - std::chrono::days(1);

  CHECK(res.check_has_begun(before_check_in) == false);
  CHECK(res.check_has_begun(check_in_sys) == true);
  CHECK(res.check_resolved(check_out_sys) == true);
  CHECK(res.get_check_out_request() == false);
  res.request_check_out(std::chrono::sys_days(check_out));
  CHECK(res.get_check_out_request() == true);
}

TEST_CASE("Reservation check-in settlement flags") {
  SingleRoom room(20.0, 106, 1, 1, 0, false, false);

  std::vector<std::unique_ptr<Guest>> guests;
  guests.push_back(std::make_unique<Family>("Robert", "Garcia"));

  std::chrono::year_month_day check_in{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(1)};
  std::chrono::year_month_day check_out{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(3)};

  Reservation res(std::move(guests), &room, check_in, check_out, true, 50);

  CHECK(res.is_checked_in() == false);
  CHECK(res.is_checkout_settled() == false);

  res.mark_checked_in();
  CHECK(res.is_checked_in() == true);

  res.mark_checkout_settled();
  CHECK(res.is_checkout_settled() == true);
}

TEST_CASE("Reservation cancellation") {
  SingleRoom room(20.0, 107, 1, 1, 0, false, false);

  std::vector<std::unique_ptr<Guest>> guests;
  guests.push_back(std::make_unique<Business>("Emma", "Martinez"));

  std::chrono::year_month_day check_in{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(10)};
  std::chrono::year_month_day check_out{
      std::chrono::year(2026), std::chrono::month(5), std::chrono::day(12)};

  Reservation res(std::move(guests), &room, check_in, check_out, false, 0);

  CHECK(res.get_is_cancelled() == false);
  res.cancel_by_guest(std::chrono::sys_days(check_in));
  CHECK(res.get_is_cancelled() == true);
}
