#include "staff.hpp"
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <ranges>

Staff::Staff(const std::string& fn, const std::string& ln, unsigned int br, unsigned int nb)
    : first_name(fn), last_name(ln), base_rate(br), night_bonus(nb) {
    if (fn.empty() || ln.empty()) throw std::invalid_argument("Names cannot be empty");
    std::ranges::fill(working_hours, Shift{0, 0});
}

std::string Staff::get_first_name() const noexcept { return first_name; }
std::string Staff::get_last_name() const noexcept { return last_name; }
unsigned int Staff::get_base_rate() const noexcept { return base_rate; }

void Staff::print_identity_and_salary() const {
    std::cout << first_name << ' ' << last_name << " [" << get_profession() << "] Salary: "
              << calculate_salary() << " USD\n";
}

void Staff::check_schedule(unsigned int day, unsigned int start_h, unsigned int end_h) const {
    if (start_h == end_h && start_h != 0) throw std::invalid_argument("Invalid shift duration");

    unsigned int len = (end_h >= start_h) ? (end_h - start_h) : (24 - start_h + end_h);
    if (len > 12) throw std::invalid_argument("Shift too long! Max 12h allowed.");

    unsigned int yesterday = (day + 6) % 7;
    unsigned int tomorrow = (day + 1) % 7;

    const Shift& prev = working_hours[yesterday];
    if (prev.start_hour != 0 || prev.end_hour != 0) {
        int gap_prev;
        if (prev.start_hour > prev.end_hour) // Night shift ends today morning
            gap_prev = static_cast<int>(start_h) - static_cast<int>(prev.end_hour);
        else // Day shift ended yesterday
            gap_prev = 24 - static_cast<int>(prev.end_hour) + static_cast<int>(start_h);

        if (gap_prev < 10) throw std::invalid_argument("Insufficient rest from yesterday (min 10h)!");
    }

    const Shift& next = working_hours[tomorrow];
    if (next.start_hour != 0 || next.end_hour != 0) {
        int gap_next;
        if (start_h > end_h) // Today's shift is a night shift
            gap_next = static_cast<int>(next.start_hour) - static_cast<int>(end_h);
        else // Today's shift ends today
            gap_next = 24 - static_cast<int>(end_h) + static_cast<int>(next.start_hour);

        if (gap_next < 10) throw std::invalid_argument("Insufficient rest before tomorrow (min 10h)!");
    }
}

// RECEPTIONIST
std::string Receptionist::get_profession() const noexcept { return "Receptionist"; }

void Receptionist::determine_schedule(unsigned int d, unsigned int t) {
    if (d > 6) throw std::out_of_range("Invalid day index");
    unsigned int s = 0, e = 0;
    if (t == 1) { s = 6; e = 14; }
    else if (t == 2) { s = 14; e = 22; }
    else if (t == 3) { s = 22; e = 6; }
    else if (t == 0) { s = 0; e = 0; }
    else throw std::invalid_argument("Unknown shift type");

    if (t != 0) check_schedule(d, s, e);
    working_hours[d] = {s, e};
}

void Receptionist::determine_schedule(unsigned int d, unsigned int s, unsigned int e) {
    if (d > 6) throw std::out_of_range("Invalid day index");
    check_schedule(d, s, e);
    working_hours[d] = {s, e};
}

unsigned int Receptionist::calculate_salary() const {
    unsigned int night_h = 0, day_h = 0;
    for (const auto& s : working_hours) {
        if (s.start_hour == 0 && s.end_hour == 0) continue; // Dzień wolny

        // Obliczamy dystans między startem a końcem (obsługuje nocki i 24h)
        unsigned int len = (s.end_hour >= s.start_hour) ? (s.end_hour - s.start_hour) : (24 - s.start_hour + s.end_hour);
        if (len == 0 && s.start_hour != 0) len = 24; // Specyficzny przypadek dla pełnej doby

        unsigned int current_h = s.start_hour;
        for (unsigned int i = 0; i < len; ++i) {
            if (current_h >= 22 || current_h < 6) night_h++;
            else day_h++;
            current_h = (current_h + 1) % 24;
        }
    }
    return (day_h * base_rate) + (night_h * (base_rate + night_bonus));
}

// MAID
std::string Maid::get_profession() const noexcept { return "Maid"; }
void Maid::determine_schedule(unsigned int d, unsigned int t) {
    if (d > 6) throw std::out_of_range("Invalid day index");
    unsigned int s = 0, e = 0;
    if (t == 1) { s = 8; e = 16; }
    else if (t == 2) { s = 12; e = 20; }
    else if (t == 0) { s = 0; e = 0; }
    else throw std::invalid_argument("Maids do not work at night!");

    if (t != 0) check_schedule(d, s, e);
    working_hours[d] = {s, e};
}
void Maid::determine_schedule(unsigned int d, unsigned int s, unsigned int e) {
    if (d > 6) throw std::out_of_range("Invalid day index");
    if (s >= 22 || s < 6 || e > 22 || e < 6 || s > e) throw std::invalid_argument("Maids do not work at night!");
    check_schedule(d, s, e);
    working_hours[d] = {s, e};
}

unsigned int Maid::calculate_salary() const {
    unsigned int h = 0;
    for (const auto& s : working_hours) {
        if (s.start_hour == 0 && s.end_hour == 0) continue;
        h += (s.end_hour - s.start_hour);
    }
    return h * base_rate;
}

// CHEF
std::string Chef::get_profession() const noexcept { return "Chef"; }
void Chef::determine_schedule(unsigned int d, unsigned int t) {
    if (d > 6) throw std::out_of_range("Invalid day index");
    unsigned int s = 0, e = 0;
    if (t == 1) { s = 5; e = 13; }
    else if (t == 2) { s = 14; e = 22; }
    else if (t == 0) { s = 0; e = 0; }
    else throw std::invalid_argument("Unknown shift type");

    if (t != 0) check_schedule(d, s, e);
    working_hours[d] = {s, e};
}
void Chef::determine_schedule(unsigned int d, unsigned int s, unsigned int e) {
    if (d > 6) throw std::out_of_range("Invalid day index");
    check_schedule(d, s, e);
    working_hours[d] = {s, e};
}
unsigned int Chef::calculate_salary() const {
    unsigned int h = 0;
    for (const auto& s : working_hours) {
        if (s.start_hour == 0 && s.end_hour == 0) continue;
        h += (s.end_hour >= s.start_hour) ? (s.end_hour - s.start_hour) : (24 - s.start_hour + s.end_hour);
    }
    return h * base_rate;
}

void Receptionist::display_info() const {
    print_identity_and_salary();
    std::cout << "  Pay model: base hourly + night premium (22:00–06:00) on covered hours.\n";
}

void Maid::display_info() const {
    print_identity_and_salary();
    std::cout << "  Pay model: straight hourly; no night shifts (day window only).\n";
}

void Chef::display_info() const {
    print_identity_and_salary();
    std::cout << "  Pay model: kitchen hourly; early/late blocks; may span long kitchen days.\n";
}