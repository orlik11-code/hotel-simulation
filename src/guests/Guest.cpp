#include "../include/guests/Guest.hpp"
#include <stdexcept>

std::string Guest::get_name() { return name; }

std::string Guest::get_surname() { return surname; }

int Guest::get_satisfaction() { return satisfaction; }

Guest::Guest(std::string name, std::string surname)
    : name(name), surname(surname) {
  if ((int)name.length() == 0) {
    throw std::invalid_argument("Name cannot be empty.");
  }
  if ((int)surname.length() == 0) {
    throw std::invalid_argument("Surname cannot be empty.");
  }
}