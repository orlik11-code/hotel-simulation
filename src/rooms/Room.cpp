//
// Created by orlik on 27.04.2026.
//
#include "../include/Room.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

// getters & setters
void Room::set_area(double area) { this->area = area; }

void Room::set_number(unsigned int number) { this->number = number; }

unsigned int Room::get_number() const { return this->number; }

unsigned int Room::get_double_beds() const { return this->double_beds; }

unsigned int Room::get_single_beds() const { return this->single_beds; }

unsigned int Room::get_floor() const { return this->floor; }

double Room::get_area() const { return this->area; }

bool Room::is_to_clean() const { return to_clean; }

bool Room::is_to_repair() const { return to_repair; }

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
void Room::destroy() { this->to_repair = true; }

void Room::make_dirty() { this->to_clean = true; }

// changing room configuration
void Room::new_config(unsigned int s_beds, unsigned int d_beds) {
  this->single_beds = s_beds;
  this->double_beds = d_beds;
}

// constructor
Room::Room(double area, unsigned int number, unsigned int floor,
           unsigned int single_beds, unsigned int double_beds, bool to_clean,
           bool to_repair)
    : area(area), number(number), floor(floor), single_beds(single_beds),
      double_beds(double_beds), to_clean(to_clean), to_repair(to_repair) {}

// checking if room is suitable for client
bool Room::check_suitability(
    const std::vector<unsigned int> &people_sleeping_together) {
  unsigned int available_s_beds = this->get_single_beds();
  unsigned int available_d_beds = this->get_double_beds();

  unsigned int p1 = 0;
  unsigned int p2 = 0;

  for (unsigned int val : people_sleeping_together) {
    if (val == 1)
      p1++;
    else if (val == 2)
      p2++;
    else
      throw std::logic_error("Only 1 or 2 people can sleep together.");
  }

  if (p1 + p2 != people_sleeping_together.size()) {
    throw std::logic_error("Invalid vector size calculation.");
  }

  if (p2 > available_d_beds) {
    return false;
  }
  available_d_beds -= p2;

  unsigned int p1_in_d_beds = (p1 < available_d_beds) ? p1 : available_d_beds;
  p1 -= p1_in_d_beds;

  return p1 <= available_s_beds;
}

bool Room::is_occupied() const { return this->occupied; }

void Room::change_occupied() { occupied ^= 1; }
