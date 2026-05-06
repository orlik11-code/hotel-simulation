#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include "staff.hpp"
#include "hotel.hpp"

TEST_CASE("Basics - Initialization and Getters") {
    Receptionist r("John", "Doe", 30, 15);
    REQUIRE(r.get_first_name() == "John");
    REQUIRE(r.get_last_name() == "Doe");
    REQUIRE(r.get_base_rate() == 30);
    REQUIRE(r.get_profession() == "Receptionist");

    // empty name check
    REQUIRE_THROWS_AS(Receptionist("", "Doe", 30, 15), std::invalid_argument);
}

TEST_CASE("BHP - Rest Periods and Time Logic") {
    Receptionist r("BHP", "Tester", 30, 0);

    // same start and end
    REQUIRE_THROWS_AS(r.determine_schedule(0, 10, 10), std::invalid_argument);

    r.determine_schedule(0, 8, 20);
    REQUIRE_NOTHROW(r.determine_schedule(1, 6, 14)); // 10h rest OK

    r.determine_schedule(6, 14, 22);
    REQUIRE_THROWS_AS(r.determine_schedule(0, 6, 14), std::invalid_argument); // 8h rest - fail
}

TEST_CASE("Role Specific Logic") {
    Maid m("Anna", "Sweep", 25, 0);
    // maid night shift type 3
    REQUIRE_THROWS_AS(m.determine_schedule(0, 3), std::invalid_argument);
    // maid custom night shift
    REQUIRE_THROWS_AS(m.determine_schedule(0, 22, 6), std::invalid_argument);
}

TEST_CASE("Salary and Bonus Math") {
    Receptionist r("Night", "Owl", 30, 15);
    r.determine_schedule(0, 6, 14);
    r.determine_schedule(1, 22, 6);
    REQUIRE(r.calculate_salary() == 600); // (8h * 30) + (8h * 45)
}

TEST_CASE("Hotel Registry and Polymorphism") {
    Hotel h(35, 28, 45, 15);
    h.add_receptionist("Rec", "Staff");
    h.add_maid("Maid", "Staff");

    // shift 3 to all
    REQUIRE_NOTHROW(h.apply_schedule_to_all(0, 3));

    // maid skips night shift -> 0$
    REQUIRE(h.get_worker(0)->calculate_salary() == 400); // Rec 8h * 50
    REQUIRE(h.get_worker(1)->calculate_salary() == 0);
}

TEST_CASE("Chef - Role Specific Logic and Shifts") {
    Chef c("Gordon", "Ramsay", 45, 0);
    REQUIRE(c.get_profession() == "Chef");

    REQUIRE_NOTHROW(c.determine_schedule(0, 1)); // 1: 5-13
    REQUIRE_NOTHROW(c.determine_schedule(1, 2)); // 2: 14-22

    // no 3rd shift
    REQUIRE_THROWS_AS(c.determine_schedule(2, 3), std::invalid_argument);

    // type 0 free day
    REQUIRE_NOTHROW(c.determine_schedule(3, 0));

    // 2 shifts * 8h = 16h * 45$ = 720$
    REQUIRE(c.calculate_salary() == 720);
}

TEST_CASE("Maid - Salary and Custom Shifts") {
    Maid m("Cleaning", "Lady", 25, 0);

    m.determine_schedule(0, 1); // 1: 8-16 (8h)
    m.determine_schedule(1, 2); // 2: 12-20 (8h)
    m.determine_schedule(2, 0); // free

    // touches night zone (23)
    REQUIRE_THROWS_AS(m.determine_schedule(3, 15, 23), std::invalid_argument);

    // 16h * 25$ = 400$
    REQUIRE(m.calculate_salary() == 400);
}

TEST_CASE("BHP - Edge Cases (Exact 10h gap and invalid shifts)") {
    Receptionist r("Edge", "Case", 30, 10);

    // exactly 10h gap OK
    r.determine_schedule(0, 6, 14); // ends 14
    REQUIRE_NOTHROW(r.determine_schedule(1, 0, 8)); // starts 0

    Receptionist r2("Edge", "Case2", 30, 10);

    // 9h gap ERROR
    r2.determine_schedule(0, 6, 15); // ends 15
    REQUIRE_THROWS_AS(r2.determine_schedule(1, 0, 8), std::invalid_argument); // starts 0

    // same start and end time (not 0)
    REQUIRE_THROWS_AS(r2.determine_schedule(3, 12, 12), std::invalid_argument);
}

TEST_CASE("Receptionist - Complex Night Bonus Math") {
    Receptionist r("Night", "Staff", 30, 15); // base 30, bonus 15

    // mixed day/night shift
    r.determine_schedule(0, 18, 2);

    // 4h day (18-22), 4h night (22-2)
    // 120$ + 180$ = 300$
    REQUIRE(r.calculate_salary() == 300);
}


TEST_CASE("Array bounds - Invalid day index") {
    Chef c("Bad", "Day", 40, 0);
    // index 7 is out of bounds (0-6)
    REQUIRE_THROWS_AS(c.determine_schedule(7, 1), std::out_of_range);
    REQUIRE_THROWS_AS(c.determine_schedule(8, 10, 18), std::out_of_range);
}

TEST_CASE("Zero shifts - zero salary") {
    Maid m("Lazy", "Bones", 20, 0);
    // no shifts applied
    REQUIRE(m.calculate_salary() == 0);
}

TEST_CASE("BHP - Max hours limit") {
    Receptionist r("Safety", "First", 30, 10);

    // 13 hr wrong
    REQUIRE_THROWS_AS(r.determine_schedule(0, 8, 21), std::invalid_argument);

    // 8 hr good
    REQUIRE_NOTHROW(r.determine_schedule(0, 8, 16));
}

TEST_CASE("Edge Case - Day Off vs Invalid Zero") {
    Receptionist r("Logic", "Check", 30, 10);

    // (0, 0) is a Day Off
    REQUIRE_NOTHROW(r.determine_schedule(0, 0, 0));

    // (12, 12) is 0 hour work
    REQUIRE_THROWS_AS(r.determine_schedule(1, 12, 12), std::invalid_argument);
}

TEST_CASE("BHP - Weekend Transition") {
    Receptionist r("BHP", "Check", 30, 10);

    // Sunday nocka ends Monday 6:00 AM
    r.determine_schedule(6, 22, 6);

    // Monday 10:00 AM start 4h rest BHP FAIL
    REQUIRE_THROWS_AS(r.determine_schedule(0, 10, 18), std::invalid_argument);
}


#include <catch2/catch_test_macros.hpp>
#include "hotel.hpp"
#include "staff.hpp"


TEST_CASE("Weekly Test - Receptionist") {
    Receptionist r("Adam", "Rec", 30, 10);

    r.determine_schedule(0, 6, 14);  // Mon: 8h Day
    r.determine_schedule(1, 14, 22); // Tue: 8h Day
    r.determine_schedule(2, 22, 6);  // Wed: 8h Night (ends Thu 06:00)

    r.determine_schedule(3, 0);      // Thu: Day off

    r.determine_schedule(4, 22, 6);  // Fri: 8h Night (ends Sat 06:00)
    r.determine_schedule(5, 0);      // Sat: Day off
    r.determine_schedule(6, 6, 14);  // Sun: 8h Day

    // MATH: Day(8+8+8=24h) * 30 + Night(8+8=16h) * 40
    // 720 + 640 = 1360
    REQUIRE(r.calculate_salary() == 1360);
}

TEST_CASE("Weekly Test - Maid") {
    Maid m("Ewa", "Nowak", 25, 0);

    for(int i=0; i<5; ++i) {
        m.determine_schedule(i, 8, 16); // 8h Day
    }
    m.determine_schedule(5, 0); // Sat off
    m.determine_schedule(6, 0); // Sun off

    // MATH: 5 days * 8h = 40h * 25 = 1000
    REQUIRE(m.calculate_salary() == 1000);
    REQUIRE_THROWS(m.determine_schedule(0, 22, 6));
}

TEST_CASE("Weekly Test - Chef") {
    // base 40
    Chef c("Robert", "Maklowicz", 40, 0);

    c.determine_schedule(0, 1); // Type 1: 5-13 (8h)
    c.determine_schedule(1, 1); // Type 1: 5-13 (8h)
    c.determine_schedule(2, 2); // Type 2: 14-22 (8h)
    c.determine_schedule(3, 2); // Type 2: 14-22 (8h)
    c.determine_schedule(4, 0); // Fri off
    c.determine_schedule(5, 0); // Sat off
    c.determine_schedule(6, 0); // Sun off

    // MATH: 4 days * 8h = 32h * 40 = 1280
    REQUIRE(c.calculate_salary() == 1280);
}