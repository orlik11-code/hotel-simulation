//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_HOTEL_SYSTEM_MANAGEMENT_FIREALARM_H
#define PROI_HOTEL_SYSTEM_MANAGEMENT_FIREALARM_H

#include "events/HotelEvent.h"
#include "hotel/Hotel.h"

class FireAlarm : public HotelEvent {
public:
  explicit FireAlarm(Hotel &hotel);
  ~FireAlarm() override;

  void apply() override;
  void clear() override;
  [[nodiscard]] const char *description() const override;

  FireAlarm(const FireAlarm &fire_alarm) = delete;
  FireAlarm &operator=(const FireAlarm &fire_alarm) = delete;
};

#endif // PROI_HOTEL_SYSTEM_MANAGEMENT_FIREALARM_H
