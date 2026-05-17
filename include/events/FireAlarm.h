//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_HOTEL_SYSTEM_MANAGEMENT_FIREALARM_H
#define PROI_HOTEL_SYSTEM_MANAGEMENT_FIREALARM_H

#include "Hotel.h"
#include "HotelEvent.h"

class FireAlarm : public HotelEvent {
public:
  FireAlarm(Hotel &hotel);
  ~FireAlarm() override;

  FireAlarm(const FireAlarm &fire_alarm) = delete;
  FireAlarm &operator=(const FireAlarm &fire_alarm) = delete;
};

#endif // PROI_HOTEL_SYSTEM_MANAGEMENT_FIREALARM_H
