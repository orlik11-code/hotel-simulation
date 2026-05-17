#include "config/hotel_loader.hpp"

#include "config/config_loader.hpp"
#include "config/file_parse.hpp"
#include "config/sim_config_parse.hpp"
#include "hotel/Hotel.h"

#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

enum class ParseSection { none, simulation, worker, schedule };

struct ScheduleEntry {
  Weekday day{Weekday::monday};
  unsigned int start_hour{0};
  unsigned int end_hour{0};
  bool night_shift{false};
};

struct WorkerDraft {
  std::optional<unsigned int> uid;
  std::optional<StaffRole> role;
  std::optional<std::string> name;
  std::optional<std::string> surname;
  std::optional<unsigned int> salary;
  std::optional<unsigned int> salary_night;
  std::optional<unsigned int> fatigue_rate;
  std::vector<ScheduleEntry> schedules;
};

[[nodiscard]] Weekday parse_weekday(const std::string &value,
                                    unsigned int line_no) {
  if (value == "monday") {
    return Weekday::monday;
  }
  if (value == "tuesday") {
    return Weekday::tuesday;
  }
  if (value == "wednesday") {
    return Weekday::wednesday;
  }
  if (value == "thursday") {
    return Weekday::thursday;
  }
  if (value == "friday") {
    return Weekday::friday;
  }
  if (value == "saturday") {
    return Weekday::saturday;
  }
  if (value == "sunday") {
    return Weekday::sunday;
  }
  throw std::invalid_argument("Line " + std::to_string(line_no) +
                              ": unknown weekday '" + value + "'");
}

[[nodiscard]] StaffRole parse_role(const std::string &value,
                                   unsigned int line_no) {
  if (value == "Receptionist") {
    return StaffRole::Receptionist;
  }
  if (value == "Housekeeper") {
    return StaffRole::Housekeeper;
  }
  if (value == "Chef") {
    return StaffRole::Chef;
  }
  if (value == "Waiter") {
    return StaffRole::Waiter;
  }
  if (value == "MaintenanceWorker") {
    return StaffRole::MaintenanceWorker;
  }
  throw std::invalid_argument("Line " + std::to_string(line_no) +
                              ": unknown role '" + value + "'");
}

[[nodiscard]] std::unique_ptr<Staff> make_worker(const WorkerDraft &draft,
                                                 unsigned int line_no) {
  if (!draft.uid || !draft.role || !draft.name || !draft.surname ||
      !draft.salary || !draft.salary_night || !draft.fatigue_rate) {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": incomplete [worker] section");
  }

  const unsigned int uid = *draft.uid;
  const std::string &first = *draft.name;
  const std::string &last = *draft.surname;
  const unsigned int sal = *draft.salary;
  const unsigned int night = *draft.salary_night;
  const unsigned int fatigue = *draft.fatigue_rate;

  switch (*draft.role) {
  case StaffRole::Receptionist:
    return std::make_unique<Receptionist>(uid, first, last, sal, night, fatigue);
  case StaffRole::Housekeeper:
    return std::make_unique<Housekeeper>(uid, first, last, sal, night, fatigue);
  case StaffRole::Chef:
    return std::make_unique<Chef>(uid, first, last, sal, night, fatigue);
  case StaffRole::Waiter:
    return std::make_unique<Waiter>(uid, first, last, sal, night, fatigue);
  case StaffRole::MaintenanceWorker:
    return std::make_unique<MaintenanceWorker>(uid, first, last, sal, night,
                                               fatigue);
  }
  throw std::invalid_argument("Line " + std::to_string(line_no) +
                              ": unsupported role");
}

void apply_worker_key(WorkerDraft &draft, const std::string &key,
                      const std::string &value, unsigned int line_no) {
  if (key == "uid") {
    draft.uid = file_parse::parse_positive(value, "uid");
  } else if (key == "role") {
    draft.role = parse_role(value, line_no);
  } else if (key == "name") {
    draft.name = value;
  } else if (key == "surname") {
    draft.surname = value;
  } else if (key == "salary") {
    draft.salary = file_parse::parse_positive(value, "salary");
  } else if (key == "salary_night") {
    draft.salary_night = file_parse::parse_positive(value, "salary_night");
  } else if (key == "fatigue_rate") {
    draft.fatigue_rate = file_parse::parse_positive(value, "fatigue_rate");
  } else {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": unknown worker key '" + key + "'");
  }
}

void apply_schedule_key(ScheduleEntry &entry, const std::string &key,
                        const std::string &value, unsigned int line_no) {
  if (key == "day") {
    entry.day = parse_weekday(value, line_no);
  } else if (key == "start") {
    entry.start_hour = file_parse::parse_non_negative(value, "start");
  } else if (key == "end") {
    entry.end_hour = file_parse::parse_non_negative(value, "end");
  } else if (key == "night") {
    entry.night_shift = (value == "1" || value == "true");
  } else {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": unknown schedule key '" + key + "'");
  }
}

void flush_worker(Hotel &hotel, WorkerDraft &draft, unsigned int line_no) {
  if (!draft.uid) {
    return;
  }

  if (hotel.check_existance(*draft.uid)) {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": duplicate worker uid " +
                                std::to_string(*draft.uid));
  }

  hotel.add_worker(make_worker(draft, line_no));

  Staff *worker = hotel.find_by_uid(*draft.uid);
  if (worker == nullptr) {
    throw std::runtime_error("Failed to register worker uid " +
                             std::to_string(*draft.uid));
  }

  for (const ScheduleEntry &entry : draft.schedules) {
    Shift shift{entry.day, entry.start_hour, entry.end_hour, entry.night_shift};
    worker->determine_schedule(entry.day, shift);
  }

  draft = WorkerDraft{};
}

} // namespace

SimConfig Hotel_loader::load(const std::string &path, Hotel &hotel) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("Cannot open hotel file: " + path);
  }

  SimConfig config;
  ParseSection section = ParseSection::none;
  WorkerDraft draft;
  ScheduleEntry current_schedule{};
  bool schedule_has_keys = false;
  std::string line;
  unsigned int line_no = 0;

  while (std::getline(in, line)) {
    ++line_no;
    if (file_parse::is_comment_or_empty(line)) {
      continue;
    }

    const std::string header = file_parse::parse_section_header(line);
    if (!header.empty()) {
      if (header == "simulation") {
        section = ParseSection::simulation;
        continue;
      }
      if (header == "worker") {
        if (schedule_has_keys) {
          draft.schedules.push_back(current_schedule);
          schedule_has_keys = false;
        }
        flush_worker(hotel, draft, line_no);
        section = ParseSection::worker;
        continue;
      }
      if (header == "schedule") {
        if (section != ParseSection::worker && !draft.uid) {
          throw std::invalid_argument("Line " + std::to_string(line_no) +
                                      ": [schedule] before [worker]");
        }
        if (schedule_has_keys) {
          draft.schedules.push_back(current_schedule);
        }
        section = ParseSection::schedule;
        current_schedule = ScheduleEntry{};
        schedule_has_keys = false;
        continue;
      }
      throw std::invalid_argument("Line " + std::to_string(line_no) +
                                  ": unknown section [" + header + "]");
    }

    const auto [key, value] = file_parse::split_key_value(line, line_no);

    if (section == ParseSection::simulation) {
      sim_config_parse::apply_key(config, key, value);
    } else if (section == ParseSection::worker) {
      apply_worker_key(draft, key, value, line_no);
    } else if (section == ParseSection::schedule) {
      apply_schedule_key(current_schedule, key, value, line_no);
      schedule_has_keys = true;
    } else {
      throw std::invalid_argument("Line " + std::to_string(line_no) +
                                  ": key=value outside a section");
    }
  }

  if (schedule_has_keys) {
    draft.schedules.push_back(current_schedule);
  }
  flush_worker(hotel, draft, line_no);

  return config;
}

SimConfig Hotel_loader::load_from_argv(int argc, char *argv[], Hotel &hotel) {
  for (int i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--hotel" && i + 1 < argc) {
      return load(argv[++i], hotel);
    }
  }
  return SimConfig{};
}

void Hotel_loader::print_usage(const char *program_name) {
  Config_loader::print_usage(program_name);
}
