#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <stdexcept>
#include <vector>

#include "staff.hpp"

namespace {
constexpr Weekday kSimWeekday = Weekday::monday;
constexpr unsigned int kSimHour = 12U;

[[nodiscard]] constexpr unsigned int task_minutes(unsigned int duration_hours)
{
    return duration_hours * 60U;
}
} // namespace

TEST_CASE("Staff getters return constructor data")
{
    Receptionist receptionist(101U, "Jan", "Kowalski", 50U, 20U, 5U);

    REQUIRE(receptionist.get_uid() == 101U);
    REQUIRE(receptionist.get_first_name() == "Jan");
    REQUIRE(receptionist.get_last_name() == "Kowalski");
    REQUIRE(receptionist.get_salary() == 50U);
    REQUIRE(receptionist.get_to_pay() == 0U);
}

TEST_CASE("Role mapping and task matching work for each class")
{
    Receptionist receptionist(1U, "A", "B", 30U, 10U, 5U);
    Housekeeper housekeeper(2U, "A", "B", 30U, 10U, 5U);
    Chef chef(3U, "A", "B", 30U, 10U, 5U);
    Waiter waiter(4U, "A", "B", 30U, 10U, 5U);
    MaintenanceWorker maintenance(5U, "A", "B", 30U, 10U, 5U);

    REQUIRE(receptionist.role() == StaffRole::Receptionist);
    REQUIRE(housekeeper.role() == StaffRole::Housekeeper);
    REQUIRE(chef.role() == StaffRole::Chef);
    REQUIRE(waiter.role() == StaffRole::Waiter);
    REQUIRE(maintenance.role() == StaffRole::MaintenanceWorker);

    REQUIRE(receptionist.can_handle(Task{TaskType::CheckIn}, kSimWeekday, kSimHour));
    REQUIRE_FALSE(receptionist.can_handle(Task{TaskType::CookMeal}, kSimWeekday, kSimHour));

    REQUIRE(housekeeper.can_handle(Task{TaskType::CleanRoom}, kSimWeekday, kSimHour));
    REQUIRE_FALSE(housekeeper.can_handle(Task{TaskType::CleanRoom, 1U, 1U, 0U, true}, kSimWeekday, kSimHour));

    REQUIRE(chef.can_handle(Task{TaskType::CookMeal}, kSimWeekday, kSimHour));
    REQUIRE(waiter.can_handle(Task{TaskType::ServeMeal}, kSimWeekday, kSimHour));
    REQUIRE(maintenance.can_handle(Task{TaskType::FixFailure}, kSimWeekday, kSimHour));
    REQUIRE_FALSE(maintenance.can_handle(Task{TaskType::FixFailure, 1U, 1U, 0U, true}, kSimWeekday, kSimHour));
}

TEST_CASE("Payment formula includes shift pay task and role bonuses")
{
    Receptionist receptionist(10U, "R", "R", 10U, 3U, 1U);
    Housekeeper housekeeper(11U, "H", "H", 10U, 3U, 1U);
    Chef chef(12U, "C", "C", 10U, 3U, 1U);
    Waiter waiter(13U, "W", "W", 10U, 3U, 1U);
    MaintenanceWorker maintenance(14U, "M", "M", 10U, 3U, 1U);

    const Task day_task{TaskType::CheckIn, 2U, 2U, 5U, false};
    receptionist.do_work(day_task, kSimWeekday, kSimHour);
    receptionist.process_tick(task_minutes(2U), kSimHour, 0U);
    REQUIRE(receptionist.get_to_pay() == 25U); // 2h shift (20) + bonus 5

    const Task clean_task{TaskType::CleanRoom, 2U, 2U, 5U, false};
    housekeeper.do_work(clean_task, kSimWeekday, kSimHour);
    housekeeper.process_tick(task_minutes(2U), kSimHour, 0U);
    REQUIRE(housekeeper.get_to_pay() == 25U);

    const Task cook_task{TaskType::CookMeal, 2U, 2U, 5U, true};
    chef.do_work(cook_task, kSimWeekday, kSimHour);
    chef.process_tick(task_minutes(2U), kSimHour, 0U);
    REQUIRE(chef.get_to_pay() == 26U); // 2h shift (20) + bonus 5 + role bonus 1

    const Task serve_task{TaskType::ServeMeal, 1U, 4U, 1U, false};
    waiter.do_work(serve_task, kSimWeekday, kSimHour);
    waiter.process_tick(task_minutes(1U), kSimHour, 0U);
    REQUIRE(waiter.get_to_pay() == 11U); // 1h shift (10) + bonus 1

    const Task fix_task{TaskType::FixFailure, 3U, 2U, 0U, false};
    maintenance.do_work(fix_task, kSimWeekday, kSimHour);
    maintenance.process_tick(task_minutes(3U), kSimHour, 0U);
    REQUIRE(maintenance.get_to_pay() == 530U); // 3h shift (30) + emergency 500
}

TEST_CASE("Night restrictions prevent housekeeper and maintenance work")
{
    Housekeeper housekeeper(20U, "H", "H", 20U, 10U, 5U);
    MaintenanceWorker maintenance(21U, "M", "M", 20U, 10U, 5U);

    const Task night_clean{TaskType::CleanRoom, 2U, 2U, 3U, true};
    const Task night_fix{TaskType::FixFailure, 2U, 2U, 3U, true};

    housekeeper.do_work(night_clean, kSimWeekday, kSimHour);
    maintenance.do_work(night_fix, kSimWeekday, kSimHour);

    REQUIRE(housekeeper.get_to_pay() == 0U);
    REQUIRE(maintenance.get_to_pay() == 0U);
}

TEST_CASE("Fatigue grows with work and can be reduced by rest")
{
    Chef chef(30U, "C", "C", 20U, 10U, 25U);

    const Task cook_task{TaskType::CookMeal, 1U, 2U, 0U, false};
    chef.do_work(cook_task, kSimWeekday, kSimHour);
    chef.process_tick(task_minutes(1U), kSimHour, 0U); // +50 fatigue (rate 25 * difficulty 2)
    REQUIRE_FALSE(chef.is_tired());

    chef.do_work(cook_task, kSimWeekday, kSimHour);
    chef.process_tick(task_minutes(1U), kSimHour, 0U); // +50 fatigue => 100
    REQUIRE(chef.is_tired());

    const unsigned int pay_before_blocked_work = chef.get_to_pay();
    chef.do_work(cook_task, kSimWeekday, kSimHour); // blocked by fatigue
    REQUIRE(chef.get_to_pay() == pay_before_blocked_work);

    chef.rest_tick(600U); // ~10 h rest in model minutes
    REQUIRE_FALSE(chef.is_tired());
}

TEST_CASE("Polymorphic collection executes overridden methods")
{
    std::vector<std::unique_ptr<Staff>> team;
    team.push_back(std::make_unique<Receptionist>(1001U, "Rec", "One", 10U, 5U, 5U));
    team.push_back(std::make_unique<Housekeeper>(1002U, "House", "One", 10U, 5U, 5U));
    team.push_back(std::make_unique<MaintenanceWorker>(1003U, "Maint", "One", 10U, 5U, 5U));

    const Task check_in{TaskType::CheckIn, 1U, 1U, 0U, false};
    const Task clean_room{TaskType::CleanRoom, 1U, 1U, 0U, false};
    const Task fix_night{TaskType::FixFailure, 1U, 1U, 0U, true};

    for (const auto& worker : team) {
        worker->do_work(check_in, kSimWeekday, kSimHour);
        worker->process_tick(task_minutes(1U), kSimHour, 0U);
        worker->do_work(clean_room, kSimWeekday, kSimHour);
        worker->process_tick(task_minutes(1U), kSimHour, 0U);
        worker->do_work(fix_night, kSimWeekday, kSimHour);
        worker->process_tick(task_minutes(1U), kSimHour, 0U);
    }

    REQUIRE(team[0]->get_to_pay() == 30U); // receptionist: 3h shift, one check-in
    REQUIRE(team[1]->get_to_pay() == 30U); // housekeeper: 3h shift, one clean-room
    REQUIRE(team[2]->get_to_pay() == 30U); // maintenance: 3h shift, night fix rejected
    REQUIRE(team[0]->get_uid() == 1001U);
    REQUIRE(team[1]->get_uid() == 1002U);
    REQUIRE(team[2]->get_uid() == 1003U);
}

TEST_CASE("Fatigue recovers only outside scheduled shift")
{
    Housekeeper housekeeper(22U, "H", "H", 10U, 3U, 5U);
    Shift day_shift{};
    day_shift.weekday = Weekday::monday;
    day_shift.start_hour = 8U;
    day_shift.end_hour = 16U;
    housekeeper.determine_schedule(Weekday::monday, day_shift);
    housekeeper.increase_fatigue(8U); // fatigue_rate 5 -> 40 points

    housekeeper.process_tick(60U, 10U, 0U);
    REQUIRE(housekeeper.get_fatigue() == 40U);

    housekeeper.process_tick(600U, 22U, 0U);
    REQUIRE(housekeeper.get_fatigue() < 40U);
}

TEST_CASE("Receptionist determine_schedule sets daytime shift on Monday")
{
    Receptionist receptionist(40U, "Rec", "One", 10U, 5U, 5U);
    receptionist.determine_schedule(Weekday::monday, Shift{Weekday::monday, 8U, 16U, false});

    REQUIRE(receptionist.is_on_shift(8U, 0U));
    REQUIRE(receptionist.is_on_shift(15U, 0U));
    REQUIRE_FALSE(receptionist.is_on_shift(7U, 0U));
    REQUIRE_FALSE(receptionist.is_on_shift(16U, 0U));
}

TEST_CASE("Receptionist determine_schedule maps Wednesday to weekday index 2")
{
    Receptionist receptionist(41U, "Rec", "Two", 10U, 5U, 5U);
    receptionist.determine_schedule(Weekday::wednesday, Shift{Weekday::wednesday, 9U, 17U, false});

    REQUIRE(receptionist.is_on_shift(10U, 2U));
    REQUIRE_FALSE(receptionist.is_on_shift(8U, 2U));
}

TEST_CASE("Receptionist determine_schedule night span sets night_shift and covers wrap hours")
{
    Receptionist receptionist(42U, "Rec", "Night", 10U, 5U, 5U);
    Shift span{Weekday::monday, 22U, 6U, false};
    receptionist.determine_schedule(Weekday::monday, span);

    REQUIRE(receptionist.is_on_shift(22U, 0U));
    REQUIRE(receptionist.is_on_shift(23U, 0U));
    REQUIRE(receptionist.is_on_shift(5U, 0U));
    REQUIRE_FALSE(receptionist.is_on_shift(12U, 0U));
}

TEST_CASE("Receptionist determine_schedule throws when start equals end and non-zero")
{
    Receptionist receptionist(43U, "Rec", "Bad", 10U, 5U, 5U);

    REQUIRE_THROWS_AS(receptionist.determine_schedule(Weekday::friday, Shift{Weekday::friday, 9U, 9U, false}),
                      std::invalid_argument);
}

TEST_CASE("Receptionist determine_schedule allows zero span as unset hours")
{
    Receptionist receptionist(44U, "Rec", "Zero", 10U, 5U, 5U);
    REQUIRE_NOTHROW(receptionist.determine_schedule(Weekday::sunday, Shift{Weekday::sunday, 0U, 0U, false}));
    REQUIRE(receptionist.is_on_shift(0U, 6U));
    REQUIRE(receptionist.is_on_shift(23U, 6U));
}