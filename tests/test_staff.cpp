#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <vector>

#include "staff.hpp"

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

    REQUIRE(receptionist.can_handle(Task{TaskType::CheckIn}));
    REQUIRE_FALSE(receptionist.can_handle(Task{TaskType::CookMeal}));

    REQUIRE(housekeeper.can_handle(Task{TaskType::CleanRoom}));
    REQUIRE_FALSE(housekeeper.can_handle(Task{TaskType::CleanRoom, 1U, 1U, 0U, true}));

    REQUIRE(chef.can_handle(Task{TaskType::CookMeal}));
    REQUIRE(waiter.can_handle(Task{TaskType::ServeMeal}));
    REQUIRE(maintenance.can_handle(Task{TaskType::FixFailure}));
    REQUIRE_FALSE(maintenance.can_handle(Task{TaskType::FixFailure, 1U, 1U, 0U, true}));
}

TEST_CASE("Payment formula includes hours night premium task and role bonuses")
{
    Receptionist receptionist(10U, "R", "R", 10U, 3U, 1U);
    Housekeeper housekeeper(11U, "H", "H", 10U, 3U, 1U);
    Chef chef(12U, "C", "C", 10U, 3U, 1U);
    Waiter waiter(13U, "W", "W", 10U, 3U, 1U);
    MaintenanceWorker maintenance(14U, "M", "M", 10U, 3U, 1U);

    const Task day_task{TaskType::CheckIn, 2U, 2U, 5U, false};
    receptionist.do_work(day_task);
    REQUIRE(receptionist.get_to_pay() == 27U); // 10*2 + 5 + role(2)

    const Task clean_task{TaskType::CleanRoom, 2U, 2U, 5U, false};
    housekeeper.do_work(clean_task);
    REQUIRE(housekeeper.get_to_pay() == 29U); // 10*2 + 5 + role(4)

    const Task cook_task{TaskType::CookMeal, 2U, 2U, 5U, true};
    chef.do_work(cook_task);
    REQUIRE(chef.get_to_pay() == 37U); // (10+3)*2 + 5 + role(6)

    const Task serve_task{TaskType::ServeMeal, 1U, 4U, 1U, false};
    waiter.do_work(serve_task);
    REQUIRE(waiter.get_to_pay() == 15U); // 10*1 + 1 + role(4)

    const Task fix_task{TaskType::FixFailure, 3U, 2U, 0U, false};
    maintenance.do_work(fix_task);
    REQUIRE(maintenance.get_to_pay() == 38U); // 10*3 + 0 + role(8)
}

TEST_CASE("Night restrictions prevent housekeeper and maintenance work")
{
    Housekeeper housekeeper(20U, "H", "H", 20U, 10U, 5U);
    MaintenanceWorker maintenance(21U, "M", "M", 20U, 10U, 5U);

    const Task night_clean{TaskType::CleanRoom, 2U, 2U, 3U, true};
    const Task night_fix{TaskType::FixFailure, 2U, 2U, 3U, true};

    housekeeper.do_work(night_clean);
    maintenance.do_work(night_fix);

    REQUIRE(housekeeper.get_to_pay() == 0U);
    REQUIRE(maintenance.get_to_pay() == 0U);
}

TEST_CASE("Fatigue grows with work and can be reduced by rest")
{
    Chef chef(30U, "C", "C", 20U, 10U, 25U);

    const Task cook_task{TaskType::CookMeal, 1U, 2U, 0U, false};
    chef.do_work(cook_task); // +50 fatigue
    REQUIRE_FALSE(chef.is_tired());

    chef.do_work(cook_task); // +50 fatigue => 100
    REQUIRE(chef.is_tired());

    const unsigned int pay_before_blocked_work = chef.get_to_pay();
    chef.do_work(cook_task); // blocked by fatigue
    REQUIRE(chef.get_to_pay() == pay_before_blocked_work);

    chef.rest_tick(); // 100 -> 75
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
        worker->do_work(check_in);
        worker->do_work(clean_room);
        worker->do_work(fix_night);
    }

    REQUIRE(team[0]->get_to_pay() == 11U); // receptionist: only check-in
    REQUIRE(team[1]->get_to_pay() == 12U); // housekeeper: only clean-room
    REQUIRE(team[2]->get_to_pay() == 0U);  // maintenance: task was night-only
    REQUIRE(team[0]->get_uid() == 1001U);
    REQUIRE(team[1]->get_uid() == 1002U);
    REQUIRE(team[2]->get_uid() == 1003U);
}