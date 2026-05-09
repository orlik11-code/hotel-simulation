#pragma once
#include "Hotel.hpp"

struct SimConfig
{
    unsigned int ticsPerDay = 24;
};

class Simulation
{
    private:
        Hotel hotel;
        SimConfig config;
        unsigned int current_tick;
        bool is_running;

    public:
        // Simulation(const SimConfig& sim_config);

        // void run();
        // void tick();

        // Hotel& get_hotel();

};