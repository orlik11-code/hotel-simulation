#pragma once

#include "staff.hpp"
#include <memory>
#include <vector>

class Workers_menagement
{
    private:
        std::vector<std::unique_ptr<Staff>> workers;

    public:
        Workers_menagement();

        [[nodiscard]] bool check_existance(unsigned int worker_uid) const;
        void add_worker(std::unique_ptr<Staff> worker);

        Staff* find_by_uid(unsigned int uid);
        std::vector<Staff*> find_by_role(StaffRole role);
        bool assign_task(unsigned int uid, const Task& task);
        [[nodiscard]] bool assign_first_available(StaffRole role, const Task& task);

};


