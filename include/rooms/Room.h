//
// Created by orlik on 27.04.2026.
//

#ifndef PROI_ZAD4_ROOM_H
#define PROI_ZAD4_ROOM_H
#include <vector>

class Room {
public:
  unsigned int get_single_beds() const;
  unsigned int get_double_beds() const;
  unsigned int get_number() const;
  unsigned int get_floor() const;
  double get_area() const;

  bool is_to_clean() const;
  bool is_to_repair() const;
  bool is_occupied() const;

  void set_price(unsigned int price);
  void set_number(unsigned int number);
  void set_area(double area);
  void clean();
  void make_dirty();
  void repair();
  void destroy();
  void new_config(unsigned int s_beds, unsigned int d_beds);
  void change_occupied();

  virtual bool
  check_suitability(const std::vector<unsigned int> &people_sleeping_together);

  Room(double area, unsigned int number, unsigned int floor,
       unsigned int single_beds, unsigned int double_beds, bool to_clean,
       bool to_repair);

  virtual ~Room() = default;

  virtual unsigned int calculateNightlyPrice() const = 0;
  virtual unsigned int calculateStaffNeeded() const = 0;

protected:
  double area;
  unsigned int number;
  unsigned int floor;
  unsigned int single_beds;
  unsigned int double_beds;
  bool to_clean;
  bool to_repair;
  bool occupied;
};
#endif // PROI_ZAD4_ROOM_H
