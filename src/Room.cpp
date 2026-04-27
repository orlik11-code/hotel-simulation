//
// Created by orlik on 27.04.2026.
//
#include "../include/Room.h"

//getters & setters
void Room::set_number(unsigned int number) {
    this->number = number;
}

void Room::set_workers_needed(unsigned int workers_needed) {
    this->workers_needed = workers_needed;
}

void Room::set_price(unsigned int price) {
    this->price = price;
}

unsigned int Room::get_number() const{
    return this->number;
}

unsigned int Room::get_workers_needed() const{
    return this->workers_needed;
}

unsigned int Room::get_price() const{
    return this->price;
}

unsigned int Room::get_double_beds() const {
    return this->double_beds;
}

unsigned int Room::get_single_beds() const {
    return this->single_beds;
}

unsigned int Room::get_floor() const {
    return this->floor;
}






