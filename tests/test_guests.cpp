#include "../include/guests/Business.hpp"
#include "../include/guests/Family.hpp"
#include "../include/guests/Guest.hpp"
#include "../include/guests/VIP.hpp"
#include "doctest.h"

TEST_CASE("Guest basic information") {
  VIP vip("John", "Doe");
  CHECK(vip.get_name() == "John");
  CHECK(vip.get_surname() == "Doe");
}

TEST_CASE("Guest initial satisfaction") {
  Business business("Jane", "Smith");
  CHECK(business.get_satisfaction() == 100);
}

TEST_CASE("VIP satisfaction reduction") {
  VIP vip("Alice", "Johnson");
  int initial_satisfaction = vip.get_satisfaction();

  vip.reduce_satisfaction(10);
  int after_first_reduction = vip.get_satisfaction();

  CHECK(after_first_reduction < initial_satisfaction);

  vip.reduce_satisfaction(5);
  int after_second_reduction = vip.get_satisfaction();

  CHECK(after_second_reduction < after_first_reduction);
}

TEST_CASE("Business satisfaction reduction") {
  Business business("Bob", "Williams");
  int initial_satisfaction = business.get_satisfaction();

  business.reduce_satisfaction(15);
  int after_reduction = business.get_satisfaction();

  CHECK(after_reduction < initial_satisfaction);
}

TEST_CASE("Family satisfaction reduction") {
  Family family("Tom", "Brown");
  int initial_satisfaction = family.get_satisfaction();

  family.reduce_satisfaction(20);
  int after_reduction = family.get_satisfaction();

  CHECK(after_reduction < initial_satisfaction);
}

TEST_CASE("VIP satisfaction never goes below zero") {
  VIP vip("Sara", "Davis");

  vip.reduce_satisfaction(150);
  CHECK(vip.get_satisfaction() >= 0);
}

TEST_CASE("Business satisfaction never goes below zero") {
  Business business("Mike", "Wilson");

  business.reduce_satisfaction(200);
  CHECK(business.get_satisfaction() >= 0);
}

TEST_CASE("Family satisfaction never goes below zero") {
  Family family("Emma", "Garcia");

  family.reduce_satisfaction(300);
  CHECK(family.get_satisfaction() >= 0);
}

TEST_CASE("Multiple guests with different types") {
  VIP vip("Guest1", "VIP");
  Business business("Guest2", "Biz");
  Family family("Guest3", "Fam");

  CHECK(vip.get_satisfaction() == 100);
  CHECK(business.get_satisfaction() == 100);
  CHECK(family.get_satisfaction() == 100);

  vip.reduce_satisfaction(10);
  business.reduce_satisfaction(20);
  family.reduce_satisfaction(30);

  CHECK(vip.get_satisfaction() < 100);
  CHECK(business.get_satisfaction() < 100);
  CHECK(family.get_satisfaction() < 100);
}
