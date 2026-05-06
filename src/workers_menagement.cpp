#include "workers_menagement.hpp"
#include "staff.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

Workers_menagement::Workers_menagement()
{
}

bool Workers_menagement::check_existance(unsigned int worker_uid) const
{
    const auto found = std::ranges::find_if(workers, [worker_uid](const std::unique_ptr<Staff>& worker) {
        return worker->get_uid() == worker_uid;
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
    auto filtered_view = workers | std::views::filter([role](const std::unique_ptr<Staff>& worker) {
        return worker->role() == role;
    }) | std::views::transform([](const std::unique_ptr<Staff>& worker) {
        return worker.get();
    });
    std::ranges::copy(filtered_view, std::back_inserter(result));

    return result;
}

bool Workers_menagement::assign_task(unsigned int uid, const Task& task)
{
    Staff* correct_worker = find_by_uid(uid);
    if (correct_worker == nullptr) {
        return false;
    }
    if (!correct_worker->can_handle(task)) {
        return false;
    }
    correct_worker->do_work(task);
    return true;
}

#include <ranges>
#include <algorithm>

bool Workers_menagement::assign_first_available(StaffRole role, const Task& task)
{

    auto available_view = workers
        | std::views::filter([&](const std::unique_ptr<Staff>& worker) {
            return worker->role() == role && !worker->is_tired() && worker->can_handle(task);
        });

    if (std::ranges::empty(available_view)) {
        return false;
    }

    auto least_tired_it = std::ranges::min_element(available_view, std::less{},
        [](const std::unique_ptr<Staff>& worker) {
            return worker->get_fatigue(); // Pamiętaj o dodaniu get_fatigue() do Staff!
        }
    );

    (*least_tired_it)->do_work(task);

    return true;
}
