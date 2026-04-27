//
// Created by orlik on 27.04.2026.
//
#include "../include/Room.h"

#include <stdexcept>

//getters & setters
void Room::set_area(double area) {
    this->area = area;
}

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

double Room::get_area() const {
    return this->area;
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

// destroying and making dirty
void Room::destroy() {
    this->to_repair = true;
}

void Room::make_dirty() {
    this->to_clean = true;
}

// changing room configuration
void Room::new_config(unsigned int s_beds, unsigned int d_beds) {
    this->single_beds = s_beds;
    this->double_beds = d_beds;
}

//constructor
Room::Room(double area, unsigned int price, unsigned int workers_needed, unsigned int number, unsigned int floor,
        unsigned int single_beds, unsigned int double_beds, bool to_clean, bool to_repair)
        : area(area),
          price(price),
          workers_needed(workers_needed),
          number(number),
          floor(floor),
          single_beds(single_beds),
          double_beds(double_beds),
          to_clean(to_clean),
          to_repair(to_repair) {
}

//checking if room is suitable for client
bool Room::check_suitability(std::vector<unsigned int> &people_sleeping_together) {
    unsigned int s_beds = this->get_single_beds();
    unsigned int d_beds = this->get_double_beds();
    for (const auto p: people_sleeping_together) {
        if (p == 1) { s_beds--;}
        else if (p == 2) { d_beds--;}
        else {throw std::out_of_range(std::to_string(p) + "people cannot sleep together!");}
    }
}




