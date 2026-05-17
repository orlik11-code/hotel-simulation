#pragma once

#include <string>

class Hotel;

class Staff_loader {
public:
  static void load_into(Hotel &hotel, const std::string &path);

  /** Reads {@code --staff <path>} from argv if present. */
  static void load_from_argv(Hotel &hotel, int argc, char *argv[]);
};
