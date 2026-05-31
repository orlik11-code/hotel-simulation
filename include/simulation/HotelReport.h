//
// Created by turbotoster on 6/1/26.
//
#ifndef PROI_HOTEL_SYSTEM_MANAGEMENT_HOTELREPORT_H
#define PROI_HOTEL_SYSTEM_MANAGEMENT_HOTELREPORT_H
#include <string>

struct HotelReport {
    int balance = 0;
    unsigned int guest_count = 0;
    unsigned int average_satisfaction = 0;  // 0 gdy brak gosci
    std::string rating_label;
};


#endif //PROI_HOTEL_SYSTEM_MANAGEMENT_HOTELREPORT_H
