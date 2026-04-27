//
// Created by orlik on 27.04.2026.
//
#include "../include/Room.h"

#include <stdexcept>

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

bool Room::is_to_clean() const {
    return to_clean;
}

bool Room::is_to_repair() const {
    return to_repair;
}

// cleaning & repairing
void Room::clean() {
    if (!(this->is_to_clean())) {
        throw(std::logic_error("Room is cleaned! You cannot clean it now."));
    }
    to_clean = false;
}

void Room::repair() {
    if (!(this->is_to_repair())) {
        throw(std::logic_error("Room is repaired! You cannot repair it now."));
    }
    to_repair = false;
}








