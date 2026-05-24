//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_HOTEL_SYSTEM_MANAGEMENT_HOTELEVENT_H
#define PROI_HOTEL_SYSTEM_MANAGEMENT_HOTELEVENT_H

#pragma once
#include "hotel/Hotel.h"

class HotelEvent {
public:
  HotelEvent(Hotel &h) : hotel(h) {};

  virtual ~HotelEvent();

  HotelEvent(const HotelEvent &) = delete;
  HotelEvent &operator=(const HotelEvent &) = delete;

  /** Włącza skutki zdarzenia (prąd, ewakuacja, awaria). */
  virtual void apply() = 0;

  /** Cofa skutki po naprawie przez konserwatora. */
  virtual void clear() = 0;

  /** Krótki opis do logu symulacji. */
  [[nodiscard]] virtual const char *description() const = 0;

protected:
  Hotel &hotel;
};

#endif // PROI_HOTEL_SYSTEM_MANAGEMENT_HOTELEVENT_H
