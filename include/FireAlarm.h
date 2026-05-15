//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_HOTEL_SYSTEM_MANAGEMENT_FIREALARM_H
#define PROI_HOTEL_SYSTEM_MANAGEMENT_FIREALARM_H

#include "Hotel.h"

class FireAlarm {
public:
  FireAlarm(Hotel &hotel);
  ~FireAlarm();

  FireAlarm(const FireAlarm &fire_alarm) = delete;
  FireAlarm &operator=(const FireAlarm &fire_alarm) = delete;

private:
  Hotel &hotel;
  bool active;
};

#endif // PROI_HOTEL_SYSTEM_MANAGEMENT_FIREALARM_H
