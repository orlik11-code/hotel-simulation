//
// Created by orlik on 15.05.2026.
//

#ifndef PROI_HOTEL_SYSTEM_MANAGEMENT_RESERVATION_H
#define PROI_HOTEL_SYSTEM_MANAGEMENT_RESERVATION_H
#include "../guests/Guest.hpp"
#include "../rooms/Room.h"
#include <chrono>
#include <string>
#include <vector>

class Reservation {
public:
  Reservation(const std::vector<Guest> &p, const Room *r,
              std::chrono::year_month_day check_in,
              std::chrono::year_month_day check_out, bool food_included,
              unsigned int food_price);
  const std::vector<Guest> &get_people() const;
  const Room &get_room() const;
  std::chrono::days get_nights() const;
  std::chrono::sys_days get_check_in() const;
  std::chrono::sys_days get_check_out() const;
  bool check_resolved(std::chrono::sys_days hotel_time) const;
  bool check_has_begun(std::chrono::sys_days hotel_time) const;
  void proceed_payment();
  void request_check_out(std::chrono::sys_days hotel_time);
  bool get_is_paid() const;
  bool get_food_included() const;
  bool get_check_out_request() const;
  unsigned int get_food_price() const;
  bool get_is_cancelled() const;
  void cancel(std::chrono::sys_days hotel_time);

  /** Recepcja zakończyła meldunek (nie mylić z check_has_begun — kalendarz). */
  [[nodiscard]] bool is_checked_in() const;
  /** Recepcja zakończyła wymeldowanie i rozliczenie po stronie hotelu. */
  [[nodiscard]] bool is_checkout_settled() const;
  void mark_checked_in();
  void mark_checkout_settled();

  ~Reservation() = default;

private:
  std::chrono::sys_days check_in;
  std::chrono::sys_days check_out;
  bool check_out_request = false;
  const Room *room;
  const std::vector<Guest> &people;
  std::chrono::days nights;
  bool is_paid = false;
  bool food_included;
  unsigned int food_price;
  bool cancelled = false;
  bool checked_in = false;
  bool checkout_settled = false;
};

#endif // PROI_HOTEL_SYSTEM_MANAGEMENT_RESERVATION_H
