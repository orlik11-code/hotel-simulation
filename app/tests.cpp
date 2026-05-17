//
// Created by orlik on 4.05.2026.
//

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../include/Apartment.h"
#include "../include/DoubleRoom.h"
#include "../include/Hotel.h"
#include "../include/SingleRoom.h"
#include "../include/TripleRoom.h"
#include "../include/doctest.h"
#include <memory>
#include <stdexcept>
#include <vector>

TEST_CASE("Room basic functionality and status management") {
  SingleRoom room(15.0, 101, 1, 1, 0, false, false);

  CHECK(room.get_number() == 101);
  CHECK(room.get_area() == 15.0);
  CHECK(room.get_floor() == 1);
  CHECK(room.is_to_clean() == false);
  CHECK(room.is_to_repair() == false);

  room.make_dirty();
  CHECK(room.is_to_clean() == true);
  room.clean();
  CHECK(room.is_to_clean() == false);
  CHECK_THROWS_AS(room.clean(), std::logic_error);

  room.destroy();
  CHECK(room.is_to_repair() == true);
  room.repair();
  CHECK(room.is_to_repair() == false);
  CHECK_THROWS_AS(room.repair(), std::logic_error);
}

TEST_CASE("SingleRoom logic and validation") {
  SingleRoom sr(12.0, 102, 0, 1, 0, false, false);
  CHECK(sr.calculateNightlyPrice() == 100);
  CHECK(sr.calculateStaffNeeded() == 1);

  std::vector<unsigned int> one_person = {1};
  std::vector<unsigned int> two_people = {2};
  CHECK(sr.check_suitability(one_person) == true);
  CHECK_THROWS_AS(sr.check_suitability(two_people), std::out_of_range);

  CHECK_THROWS_AS(SingleRoom(12.0, 103, 0, 1, 1, false, false),
                  std::logic_error);
}

TEST_CASE("DoubleRoom logic and validation") {
  DoubleRoom dr(20.0, 201, 2, 0, 1, false, false);
  CHECK(dr.calculateNightlyPrice() == 170 + (15 * 2));
  CHECK(dr.calculateStaffNeeded() == 2);

  std::vector<unsigned int> couple = {2};
  std::vector<unsigned int> group = {1, 1, 1};
  CHECK(dr.check_suitability(couple) == true);
  CHECK_THROWS_AS(dr.check_suitability(group), std::out_of_range);

  CHECK_THROWS_AS(DoubleRoom(20.0, 202, 1, 2, 1, false, false),
                  std::logic_error);
}

TEST_CASE("TripleRoom logic and validation") {
  TripleRoom tr(25.0, 301, 1, 1, 1, false, false);
  CHECK(tr.calculateNightlyPrice() == 220 + 20);
  CHECK(tr.calculateStaffNeeded() == 2);

  std::vector<unsigned int> family = {2, 1};
  CHECK(tr.check_suitability(family) == true);

  CHECK_THROWS_AS(TripleRoom(25.0, 302, 1, 2, 2, false, false),
                  std::logic_error);
}

TEST_CASE("Apartment logic") {
  Apartment ap(45.0, 401, 5, 2, 1, false, false);
  unsigned int expected_price = 170 + (10 * 3) + (20 * 5);
  CHECK(ap.calculateNightlyPrice() == expected_price);
  CHECK(ap.calculateStaffNeeded() == (3 / 3) + 1);
}

TEST_CASE("Hotel collection and polymorphism") {
  Hotel hotel;
  hotel.addRoom(std::make_unique<SingleRoom>(10.0, 1, 0, 1, 0, false, false));
  hotel.addRoom(std::make_unique<Apartment>(50.0, 2, 10, 2, 2, false, false));

  SUBCASE("Removing rooms") {
    hotel.removeRoom(1);
    hotel.removeRoom(2);
  }
}