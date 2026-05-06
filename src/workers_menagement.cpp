#include "workers_menagement.hpp"
#include "staff.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <ranges>
#include <utility>

Workers_menagement::Workers_menagement()
{
}

bool Workers_menagement::check_existance(StaffRole mode, unsigned int worker_uid) const
{
    const auto found = std::ranges::find_if(workers, [mode, worker_uid](const std::unique_ptr<Staff>& worker) {
        return worker->role() == mode && worker->get_uid() == worker_uid;
    });
    return found != workers.end();
}

void Workers_menagement::add_worker(std::unique_ptr<Staff> worker)
{
    if (worker) {
        workers.push_back(std::move(worker));
    }
}

Staff* Workers_menagement::find_by_uid(unsigned int uid)
{
    const auto found = std::ranges::find_if(workers, [&](const std::unique_ptr<Staff>& worker) {
        return worker->get_uid() == uid;
    });

    if (found == workers.end()) {
        return nullptr;
    }
    return found->get();
}

std::vector<Staff*> Workers_menagement::find_by_role(StaffRole role)
{
    std::vector<Staff*> result;
    auto filtered_view = workers
    | std::views::filter([role](const std::unique_ptr<Staff>& worker) {
        return worker->role() == role;
    })
    | std::views::transform([](const std::unique_ptr<Staff>& worker) {
        return worker.get();
    });
    std::ranges::copy(filtered_view, std::back_inserter(result));

    return result;
}


// bool assign_task(unsigned int uid, const Task& task)
// {

// }
