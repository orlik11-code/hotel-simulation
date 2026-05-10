#pragma once

#include "staff.hpp"
#include <memory>
#include <vector>

class Hotel
{
    private:
        std::vector<std::unique_ptr<Staff>> workers;

    public:
        Hotel();

        [[nodiscard]] bool check_existance(unsigned int worker_uid) const;
        void add_worker(std::unique_ptr<Staff> worker);
        /** elapsed_minutes — czas jednego ticku symulacji (np. z Simulation::minutes_per_tick()). */
        void tick(unsigned int elapsed_minutes);

        Staff* find_by_uid(unsigned int uid);
        std::vector<Staff*> find_by_role(StaffRole role);
        bool assign_task(unsigned int uid, const Task& task, unsigned int hour_of_day, unsigned int weekday_index);
        [[nodiscard]] bool assign_first_available(StaffRole role, const Task& task, unsigned int hour_of_day,
                                                  unsigned int weekday_index);

};


