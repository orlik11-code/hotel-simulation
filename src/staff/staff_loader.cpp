#include "staff/staff_loader.hpp"

#include "config/hotel_loader.hpp"

void Staff_loader::load_into(Hotel &hotel, const std::string &path) {
  (void)Hotel_loader::load(path, hotel);
}

void Staff_loader::load_from_argv(Hotel &hotel, int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--staff" && i + 1 < argc) {
      (void)Hotel_loader::load(argv[++i], hotel);
      return;
    }
  }
}
