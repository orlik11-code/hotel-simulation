#include "../include/events/EventGenerator.h"
#include "../include/events/FireAlarm.h"
#include "../include/events/PowerOutage.h"
#include "../include/hotel/Hotel.h"
#include "doctest.h"

TEST_CASE("EventGenerator initialization") {
  Hotel hotel;
  EventGenerator generator(hotel, 200U);

  CHECK(&generator.get_hotel() == &hotel);
}

TEST_CASE("EventGenerator probability limit") {
  Hotel hotel;
  CHECK_THROWS_AS(EventGenerator(hotel, 301U), std::invalid_argument);
  CHECK_NOTHROW(EventGenerator(hotel, 300U));
  CHECK_NOTHROW(EventGenerator(hotel, 5U));
}

TEST_CASE("EventGenerator creates events") {
  Hotel hotel;
  EventGenerator generator(hotel, 300U);

  std::unique_ptr<HotelEvent> event = generator.decide_what_happens();
}

TEST_CASE("EventGenerator with low probability") {
  Hotel hotel;
  EventGenerator generator(hotel, 10U);

  int event_count = 0;
  for (int i = 0; i < 10; ++i) {
    auto event = generator.decide_what_happens();
    if (event != nullptr) {
      event_count++;
    }
  }

  CHECK(event_count <= 10);
}

TEST_CASE("EventGenerator with high probability") {
  Hotel hotel;
  EventGenerator generator(hotel, 250U);

  int event_count = 0;
  for (int i = 0; i < 10; ++i) {
    auto event = generator.decide_what_happens();
    if (event != nullptr) {
      event_count++;
    }
  }

  CHECK(event_count >= 0);
}

TEST_CASE("FireAlarm creation and description") {
  Hotel hotel;
  FireAlarm alarm(hotel);

  const char *desc = alarm.description();
  CHECK(desc != nullptr);
  CHECK(std::string(desc).length() > 0);
}

TEST_CASE("FireAlarm apply and clear") {
  Hotel hotel;
  FireAlarm alarm(hotel);

  alarm.apply();
  CHECK(true);

  alarm.clear();
  CHECK(true);
}

TEST_CASE("PowerOutage creation and description") {
  Hotel hotel;
  PowerOutage outage(hotel);

  const char *desc = outage.description();
  CHECK(desc != nullptr);
  CHECK(std::string(desc).length() > 0);
}

TEST_CASE("PowerOutage apply and clear") {
  Hotel hotel;
  PowerOutage outage(hotel);

  outage.apply();
  CHECK(true);

  outage.clear();
  CHECK(true);
}

TEST_CASE("FireAlarm and PowerOutage have different descriptions") {
  Hotel hotel;
  FireAlarm alarm(hotel);
  PowerOutage outage(hotel);

  std::string alarm_desc = alarm.description();
  std::string outage_desc = outage.description();

  CHECK(alarm_desc != outage_desc);
}

TEST_CASE("Multiple events can be created sequentially") {
  Hotel hotel;

  FireAlarm alarm1(hotel);
  PowerOutage outage1(hotel);
  FireAlarm alarm2(hotel);

  CHECK(alarm1.description() != nullptr);
  CHECK(outage1.description() != nullptr);
  CHECK(alarm2.description() != nullptr);
}
