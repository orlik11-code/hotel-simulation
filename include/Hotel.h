#ifndef HOTEL_H
#define HOTEL_H

#include "Room.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

class Hotel {
public:
  void addRoom(std::unique_ptr<Room> room) { rooms.push_back(std::move(room)); }

  void removeRoom(unsigned int roomNumber) {
    auto it = std::remove_if(rooms.begin(), rooms.end(),
                             [roomNumber](const std::unique_ptr<Room> &r) {
                               return r->get_number() == roomNumber;
                             });
    rooms.erase(it, rooms.end());
  }

  void showReport() const {
    for (const auto &room : rooms) {
      std::cout << "Pokoj nr: " << room->get_number()
                << " | Cena: " << room->calculateNightlyPrice()
                << " | Obsluga: " << room->calculateStaffNeeded() << std::endl;
    }
  }

private:
  std::vector<std::unique_ptr<Room>> rooms;
};

#endif