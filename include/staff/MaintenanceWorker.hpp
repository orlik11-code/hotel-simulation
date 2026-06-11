#pragma once

#include "staff/Staff.hpp"

/** Maintenance worker: fixes failures and technical issues. */
class MaintenanceWorker final : public Staff {
protected:
    [[nodiscard]] unsigned int max_shift_duration_hours() const noexcept override;
    [[nodiscard]] unsigned int calculate_task_payment(const Task& task,
                                                      unsigned int role_bonus) const noexcept override;

public:
    using Staff::Staff;

    [[nodiscard]] StaffRole role() const noexcept override;
    [[nodiscard]] bool can_handle(const Task& task, Weekday weekday, unsigned int hour_of_day) const noexcept override;
    void do_work(const Task& task, Weekday weekday, unsigned int hour_of_day) override;
    void determine_schedule(Weekday day, Shift shift_type) override;
    unsigned int receive_bonus() const noexcept override;
};
