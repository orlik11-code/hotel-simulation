#include "Simulation.hpp"
#include "Hotel.hpp"
#include <iostream>

Simulation::Simulation(const SimConfig& new_sim_config)
{
    config = new_sim_config;
    is_running = true;
    current_tick = 0;
    hotel = Hotel();
}

Hotel& Simulation::get_hotel()
{
    return hotel;
}

void Simulation::tick()
{
    current_tick++;
    hotel.tick();

    std::cout << "[Czas] Minal 1 tick. Obecny tick: " << current_tick << " (godz. " << hour_of_day()
              << ", dzien_tygodnia " << weekday() << ")\n";
}

unsigned int Simulation::get_current_tick() const noexcept
{
    return current_tick;
}

unsigned int Simulation::hour_of_day() const noexcept
{
    if (current_tick == 0U) {
        return 0U;
    }
    return (current_tick - 1U) % config.ticsPerDay;
}

unsigned int Simulation::weekday() const noexcept
{
    if (current_tick == 0U) {
        return 0U;
    }
    return ((current_tick - 1U) / config.ticsPerDay) % 7U;
}