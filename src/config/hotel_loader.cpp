#include "config/hotel_loader.hpp"

#include "config/config_loader.hpp"
#include "config/file_parse.hpp"
#include "config/sim_config_parse.hpp"
#include "hotel/Hotel.h"

#include "booking/Reservation.h"
#include "guests/Business.hpp"
#include "guests/Family.hpp"
#include "guests/VIP.hpp"
#include "rooms/Apartment.h"
#include "rooms/DoubleRoom.h"
#include "rooms/SingleRoom.h"
#include "rooms/TripleRoom.h"
#include "meal/MealTypes.hpp"
#include "simulation/SimConfig.hpp"

#include <fstream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

enum class RoomType { Single, Double, Triple, Apartment };

enum class ParseSection {
  none,
  simulation,
  worker,
  schedule,
  room,
  reservation,
  reservation_guest,
  meal_service
};

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

struct RoomDraft {
  std::optional<RoomType> type;
  std::optional<unsigned int> number;
  std::optional<unsigned int> floor;
  std::optional<double> area;
  std::optional<unsigned int> single_beds;
  std::optional<unsigned int> double_beds;
  bool to_clean = false;
  bool to_repair = false;
};

struct GuestDraft {
  std::optional<std::string> type;
  std::optional<std::string> name;
  std::optional<std::string> surname;
};

struct ReservationDraft {
  std::optional<unsigned int> room_number;
  std::optional<std::chrono::year_month_day> check_in;
  std::optional<std::chrono::year_month_day> check_out;
  bool food_included = false;
  unsigned int food_price = 0U;
  unsigned int hour_check_in = 12U;
  unsigned int minute_check_in = 0U;
  unsigned int hour_check_out = 7U;
  unsigned int minute_check_out = 30U;
  std::vector<GuestDraft> guests;
};

struct MealServiceDraft {
  std::optional<MealType> type;
  std::optional<unsigned int> start_hour;
  std::optional<unsigned int> end_hour;
};

struct LoadContext {
  std::unordered_map<unsigned int, Room *> room_by_number;
};

[[nodiscard]] bool parse_bool(const std::string &value) {
  return value == "1" || value == "true" || value == "yes";
}

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

[[nodiscard]] RoomType parse_room_type(const std::string &value,
                                       unsigned int line_no) {
  if (value == "Single") {
    return RoomType::Single;
  }
  if (value == "Double") {
    return RoomType::Double;
  }
  if (value == "Triple") {
    return RoomType::Triple;
  }
  if (value == "Apartment") {
    return RoomType::Apartment;
  }
  throw std::invalid_argument("Line " + std::to_string(line_no) +
                              ": unknown room type '" + value + "'");
}

[[nodiscard]] MealType parse_meal(const std::string &value, unsigned int line_no) {
  if (value == "breakfast") {
    return MealType::breakfast;
  }
  if (value == "dinner") {
    return MealType::dinner;
  }
  throw std::invalid_argument("Line " + std::to_string(line_no) +
                              ": unknown meal type '" + value + "'");
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

[[nodiscard]] MealServiceWindow make_meal(const MealServiceDraft &draft,
                                          unsigned int line_no) {
  if (!draft.type || !draft.end_hour || !draft.start_hour) {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": incomplete [meal_service] section");
  }

  return MealServiceWindow{*draft.type, *draft.start_hour, *draft.end_hour};
}

[[nodiscard]] std::unique_ptr<Room> make_room(const RoomDraft &draft,
                                              unsigned int line_no) {
  if (!draft.type || !draft.number || !draft.floor || !draft.area) {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": incomplete [room] section");
  }

  const double area = *draft.area;
  const unsigned int number = *draft.number;
  const unsigned int floor = *draft.floor;
  const bool to_clean = draft.to_clean;
  const bool to_repair = draft.to_repair;

  switch (*draft.type) {
  case RoomType::Single: {
    const unsigned int single_beds = draft.single_beds.value_or(1U);
    const unsigned int double_beds = draft.double_beds.value_or(0U);
    return std::make_unique<SingleRoom>(area, number, floor, single_beds,
                                        double_beds, to_clean, to_repair);
  }
  case RoomType::Double: {
    if (!draft.single_beds || !draft.double_beds) {
      throw std::invalid_argument("Line " + std::to_string(line_no) +
                                  ": Double room requires single_beds and "
                                  "double_beds");
    }
    return std::make_unique<DoubleRoom>(area, number, floor, *draft.single_beds,
                                       *draft.double_beds, to_clean,
                                       to_repair);
  }
  case RoomType::Triple: {
    if (!draft.single_beds || !draft.double_beds) {
      throw std::invalid_argument("Line " + std::to_string(line_no) +
                                  ": Triple room requires single_beds and "
                                  "double_beds");
    }
    return std::make_unique<TripleRoom>(area, number, floor, *draft.single_beds,
                                      *draft.double_beds, to_clean, to_repair);
  }
  case RoomType::Apartment: {
    if (!draft.single_beds || !draft.double_beds) {
      throw std::invalid_argument("Line " + std::to_string(line_no) +
                                  ": Apartment requires single_beds and "
                                  "double_beds");
    }
    return std::make_unique<Apartment>(area, number, floor, *draft.single_beds,
                                      *draft.double_beds, to_clean, to_repair);
  }
  }
  throw std::invalid_argument("Line " + std::to_string(line_no) +
                              ": unsupported room type");
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

void apply_meal_service_key(MealServiceDraft &draft, const std::string &key,
                            const std::string &value, unsigned int line_no) {
  if (key == "start_hour") {
    draft.start_hour = file_parse::parse_positive(value, "start_hour");
  } else if (key == "type") {
    draft.type = parse_meal(value, line_no);
  } else if (key == "end_hour") {
    draft.end_hour = file_parse::parse_positive(value, "end_hour");
  } else {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": unknown meal_service key '" + key + "'");
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
    entry.night_shift = parse_bool(value);
  } else {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": unknown schedule key '" + key + "'");
  }
}

void apply_room_key(RoomDraft &draft, const std::string &key,
                    const std::string &value, unsigned int line_no) {
  if (key == "type") {
    draft.type = parse_room_type(value, line_no);
  } else if (key == "number") {
    draft.number = file_parse::parse_positive(value, "number");
  } else if (key == "floor") {
    draft.floor = file_parse::parse_non_negative(value, "floor");
  } else if (key == "area") {
    draft.area = std::stod(value);
  } else if (key == "single_beds") {
    draft.single_beds = file_parse::parse_non_negative(value, "single_beds");
  } else if (key == "double_beds") {
    draft.double_beds = file_parse::parse_non_negative(value, "double_beds");
  } else if (key == "to_clean") {
    draft.to_clean = parse_bool(value);
  } else if (key == "to_repair") {
    draft.to_repair = parse_bool(value);
  } else {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": unknown room key '" + key + "'");
  }
}

void apply_reservation_key(ReservationDraft &draft, const std::string &key,
                           const std::string &value, unsigned int line_no) {
  if (key == "room_number") {
    draft.room_number = file_parse::parse_positive(value, "room_number");
  } else if (key == "check_in") {
    draft.check_in = file_parse::parse_date(value, "check_in");
  } else if (key == "check_out") {
    draft.check_out = file_parse::parse_date(value, "check_out");
  } else if (key == "food_included") {
    draft.food_included = parse_bool(value);
  } else if (key == "food_price") {
    draft.food_price = file_parse::parse_non_negative(value, "food_price");
  } else if (key == "hour_check_in") {
    draft.hour_check_in =
        file_parse::parse_non_negative(value, "hour_check_in");
    if (draft.hour_check_in > 23U) {
      throw std::invalid_argument("Line " + std::to_string(line_no) +
                                  ": hour_check_in must be 0-23");
    }
  } else if (key == "minute_check_in") {
    draft.minute_check_in =
        file_parse::parse_non_negative(value, "minute_check_in");
    if (draft.minute_check_in > 59U) {
      throw std::invalid_argument("Line " + std::to_string(line_no) +
                                  ": minute_check_in must be 0-59");
    }
  } else if (key == "hour_check_out") {
    draft.hour_check_out =
        file_parse::parse_non_negative(value, "hour_check_out");
    if (draft.hour_check_out > 23U) {
      throw std::invalid_argument("Line " + std::to_string(line_no) +
                                  ": hour_check_out must be 0-23");
    }
  } else if (key == "minute_check_out") {
    draft.minute_check_out =
        file_parse::parse_non_negative(value, "minute_check_out");
    if (draft.minute_check_out > 59U) {
      throw std::invalid_argument("Line " + std::to_string(line_no) +
                                  ": minute_check_out must be 0-59");
    }
  } else {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": unknown reservation key '" + key + "'");
  }
}

void apply_guest_key(GuestDraft &draft, const std::string &key,
                     const std::string &value, unsigned int line_no) {
  if (key == "type") {
    draft.type = value;
  } else if (key == "name") {
    draft.name = value;
  } else if (key == "surname") {
    draft.surname = value;
  } else {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": unknown reservation_guest key '" + key +
                                "'");
  }
}

void append_guest_draft(ReservationDraft &reservation, GuestDraft &guest,
                        bool guest_has_keys, unsigned int line_no) {
  if (!guest_has_keys) {
    return;
  }
  if (!guest.type || !guest.name || !guest.surname) {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": incomplete [reservation_guest] section");
  }
  reservation.guests.push_back(guest);
  guest = GuestDraft{};
}

void add_guest_to_people(std::vector<std::unique_ptr<Guest>> &people,
                         const GuestDraft &guest, unsigned int line_no) {
  if (!guest.type || !guest.name || !guest.surname) {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": incomplete guest entry");
  }

  const std::string &type = *guest.type;
  const std::string &first = *guest.name;
  const std::string &last = *guest.surname;

  if (type == "Business") {
    people.push_back(std::make_unique<Business>(first, last));
    return;
  }
  if (type == "VIP") {
    people.push_back(std::make_unique<VIP>(first, last));
    return;
  }
  if (type == "Family") {
    people.push_back(std::make_unique<Family>(first, last));
    return;
  }
  throw std::invalid_argument("Line " + std::to_string(line_no) +
                              ": unknown guest type '" + type + "'");
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

void flush_room(Hotel &hotel, LoadContext &ctx, RoomDraft &draft,
                unsigned int line_no) {
  if (!draft.number) {
    return;
  }

  const unsigned int room_number = *draft.number;
  if (ctx.room_by_number.contains(room_number)) {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": duplicate room number " +
                                std::to_string(room_number));
  }

  std::unique_ptr<Room> room = make_room(draft, line_no);
  Room *room_ptr = room.get();
  hotel.addRoom(std::move(room));
  ctx.room_by_number[room_number] = room_ptr;
  draft = RoomDraft{};
}

void flush_meal_service(SimConfig &config, MealServiceDraft &draft,
                        unsigned int line_no) {
  if (!draft.type) {
    return;
  }

  const MealServiceWindow window = make_meal(draft, line_no);
  if (window.start_hour > 24U || window.end_hour > 24U) {
    throw std::invalid_argument("only 24h in dobe!");
  }
  config.meal_windows.push_back(window);
  draft = MealServiceDraft{};
}


void flush_reservation(Hotel &hotel, LoadContext &ctx, ReservationDraft &draft,
                       unsigned int line_no) {
  if (!draft.room_number) {
    return;
  }

  if (!draft.check_in || !draft.check_out) {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": incomplete [reservation] section");
  }
  if (draft.guests.empty()) {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": reservation requires at least one "
                                "[reservation_guest]");
  }
  if (*draft.check_in >= *draft.check_out) {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": check_out must be after check_in");
  }

  const auto room_it = ctx.room_by_number.find(*draft.room_number);
  if (room_it == ctx.room_by_number.end()) {
    throw std::invalid_argument("Line " + std::to_string(line_no) +
                                ": unknown room_number " +
                                std::to_string(*draft.room_number));
  }

  std::vector<std::unique_ptr<Guest>> &people = hotel.emplace_guest_list();
  for (const GuestDraft &guest : draft.guests) {
    add_guest_to_people(people, guest, line_no);
  }

  Room *room = room_it->second;
  hotel.add_reservation(std::make_unique<Reservation>(
      std::move(people), room, *draft.check_in, *draft.check_out,
      draft.food_included, draft.food_price, draft.hour_check_in,
      draft.minute_check_in, draft.hour_check_out, draft.minute_check_out));

  draft = ReservationDraft{};
}

void flush_open_schedule(WorkerDraft &worker_draft, ScheduleEntry &schedule,
                         bool &schedule_has_keys) {
  if (schedule_has_keys) {
    worker_draft.schedules.push_back(schedule);
    schedule = ScheduleEntry{};
    schedule_has_keys = false;
  }
}

void flush_before_section_change(SimConfig &config, Hotel &hotel,
                                 LoadContext &ctx, WorkerDraft &worker_draft,
                                 ScheduleEntry &schedule, bool &schedule_has_keys,
                                 RoomDraft &room_draft,
                                 ReservationDraft &reservation_draft,
                                 GuestDraft &guest_draft, bool &guest_has_keys,
                                 MealServiceDraft &meal_draft,
                                 unsigned int line_no) {
  flush_open_schedule(worker_draft, schedule, schedule_has_keys);
  flush_worker(hotel, worker_draft, line_no);
  append_guest_draft(reservation_draft, guest_draft, guest_has_keys, line_no);
  guest_has_keys = false;
  flush_reservation(hotel, ctx, reservation_draft, line_no);
  flush_room(hotel, ctx, room_draft, line_no);
  flush_meal_service(config, meal_draft, line_no);
}

} // namespace

SimConfig Hotel_loader::load(const std::string &path, Hotel &hotel) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("Cannot open hotel file: " + path);
  }

  SimConfig config;
  LoadContext ctx;
  ParseSection section = ParseSection::none;
  WorkerDraft worker_draft;
  ScheduleEntry current_schedule{};
  bool schedule_has_keys = false;
  RoomDraft room_draft;
  ReservationDraft reservation_draft;
  GuestDraft guest_draft;
  bool guest_has_keys = false;
  MealServiceDraft meal_draft;
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
        flush_before_section_change(config, hotel, ctx, worker_draft,
                                    current_schedule, schedule_has_keys,
                                    room_draft, reservation_draft, guest_draft,
                                    guest_has_keys, meal_draft, line_no);
        section = ParseSection::simulation;
        continue;
      }
      if (header == "worker") {
        flush_before_section_change(config, hotel, ctx, worker_draft,
                                    current_schedule, schedule_has_keys,
                                    room_draft, reservation_draft, guest_draft,
                                    guest_has_keys, meal_draft, line_no);
        section = ParseSection::worker;
        continue;
      }
      if (header == "schedule") {
        if (section != ParseSection::worker && !worker_draft.uid) {
          throw std::invalid_argument("Line " + std::to_string(line_no) +
                                      ": [schedule] before [worker]");
        }
        flush_open_schedule(worker_draft, current_schedule, schedule_has_keys);
        section = ParseSection::schedule;
        current_schedule = ScheduleEntry{};
        schedule_has_keys = false;
        continue;
      }
      if (header == "room") {
        flush_before_section_change(config, hotel, ctx, worker_draft,
                                    current_schedule, schedule_has_keys,
                                    room_draft, reservation_draft, guest_draft,
                                    guest_has_keys, meal_draft, line_no);
        section = ParseSection::room;
        continue;
      }
      if (header == "reservation") {
        flush_before_section_change(config, hotel, ctx, worker_draft,
                                    current_schedule, schedule_has_keys,
                                    room_draft, reservation_draft, guest_draft,
                                    guest_has_keys, meal_draft, line_no);
        section = ParseSection::reservation;
        continue;
      }
      if (header == "reservation_guest") {
        if (section != ParseSection::reservation &&
            section != ParseSection::reservation_guest) {
          throw std::invalid_argument(
              "Line " + std::to_string(line_no) +
              ": [reservation_guest] must follow [reservation]");
        }
        append_guest_draft(reservation_draft, guest_draft, guest_has_keys,
                           line_no);
        guest_has_keys = false;
        section = ParseSection::reservation_guest;
        continue;
      }
      if (header == "meal_service") {
        flush_before_section_change(config, hotel, ctx, worker_draft,
                                    current_schedule, schedule_has_keys,
                                    room_draft, reservation_draft, guest_draft,
                                    guest_has_keys, meal_draft, line_no);
        section = ParseSection::meal_service;
        continue;
      }
      throw std::invalid_argument("Line " + std::to_string(line_no) +
                                  ": unknown section [" + header + "]");
    }

    const auto [key, value] = file_parse::split_key_value(line, line_no);

    if (section == ParseSection::simulation) {
      sim_config_parse::apply_key(config, key, value);
    } else if (section == ParseSection::worker) {
      apply_worker_key(worker_draft, key, value, line_no);
    } else if (section == ParseSection::schedule) {
      apply_schedule_key(current_schedule, key, value, line_no);
      schedule_has_keys = true;
    } else if (section == ParseSection::room) {
      apply_room_key(room_draft, key, value, line_no);
    } else if (section == ParseSection::reservation) {
      apply_reservation_key(reservation_draft, key, value, line_no);
    } else if (section == ParseSection::reservation_guest) {
      apply_guest_key(guest_draft, key, value, line_no);
      guest_has_keys = true;
    } else if (section == ParseSection::meal_service) {
      apply_meal_service_key(meal_draft, key, value, line_no);
    } else {
      throw std::invalid_argument("Line " + std::to_string(line_no) +
                                  ": key=value outside a section");
    }
  }

  flush_before_section_change(config, hotel, ctx, worker_draft, current_schedule,
                              schedule_has_keys, room_draft, reservation_draft,
                              guest_draft, guest_has_keys, meal_draft, line_no);

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
