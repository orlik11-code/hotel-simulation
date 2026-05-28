#include "config/config_loader.hpp"
#include "config/hotel_loader.hpp"
#include "simulation/Simulation.hpp"
#include "staff/staff_loader.hpp"

#include <iostream>


int main(int argc, char *argv[]) {
  try {
    Simulation simulation({});
    Hotel &hotel = simulation.get_hotel();

    SimConfig config = Hotel_loader::load_from_argv(argc, argv, hotel);
    Staff_loader::load_from_argv(hotel, argc, argv);
    config = Config_loader::load(argc, argv, config);

    simulation.set_config(config);
    simulation.set_live_console_output(true);
    simulation.run();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << '\n';
    Config_loader::print_usage(argc > 0 ? argv[0] : "HotelApp");
    return 1;
  }

  return 0;
}
