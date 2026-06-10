#pragma once

#include "booking/Reservation.h"
#include "meal/MealTypes.hpp"

#include <chrono>

enum class MealPhase { Cooking, Ready };

struct PendingMealTask {
  Reservation *reservation = nullptr;
  MealType meal_type{};
  MealPhase phase = MealPhase::Cooking;
  unsigned int worker_uid = 0U;
};

/** Guest meal attendance plan (once per day per meal slot). */
struct MealAttendancePlan {
  Reservation *reservation = nullptr;
  MealType meal_type{};
  std::chrono::sys_days day{};
  bool will_attend = false;
  unsigned int planned_arrival_hour = 0U;
};

struct MealServedRecord {
  Reservation *reservation = nullptr;
  MealType meal_type{};
  std::chrono::sys_days day{};
};

struct MealNoShowRecord {
  Reservation *reservation = nullptr;
  MealType meal_type{};
  std::chrono::sys_days day{};
};

struct PendingReceptionTask {
  Reservation *reservation = nullptr;
  bool is_checkout = false;
  unsigned int worker_uid = 0U;
};
