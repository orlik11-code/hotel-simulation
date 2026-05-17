//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_HOTEL_SYSTEM_MANAGEMENT_HOTELEVENT_H
#define PROI_HOTEL_SYSTEM_MANAGEMENT_HOTELEVENT_H

#pragma once
#include "Hotel.h"

class HotelEvent {
public:
  HotelEvent(Hotel &h) : hotel(h) {};

  virtual ~HotelEvent();

  HotelEvent(const HotelEvent &) = delete;
  HotelEvent &operator=(const HotelEvent &) = delete;

protected:
  Hotel &hotel;
};

#endif // PROI_HOTEL_SYSTEM_MANAGEMENT_HOTELEVENT_H
