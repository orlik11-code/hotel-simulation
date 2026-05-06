#pragma once
#include <array>
#include <string>

struct Shift {
    unsigned int start_hour = 0;
    unsigned int end_hour = 0;
};

class Staff {
protected:
    std::string first_name;
    std::string last_name;
    std::array<Shift, 7> working_hours;
    unsigned int base_rate;
    unsigned int night_bonus;

    void print_identity_and_salary() const;

public:
    Staff(const std::string& first_name, const std::string& last_name, unsigned int base_rate, unsigned int night_bonus);
    virtual ~Staff() = default;

    [[nodiscard]] std::string get_first_name() const noexcept;
    [[nodiscard]] std::string get_last_name() const noexcept;
    [[nodiscard]] unsigned int get_base_rate() const noexcept;

    virtual unsigned int calculate_salary() const = 0;
    virtual void display_info() const = 0;
    [[nodiscard]] virtual std::string get_profession() const = 0;

    void check_schedule(unsigned int day_index, unsigned int start_hour, unsigned int end_hour) const;
    virtual void determine_schedule(unsigned int day_index, unsigned int shift_type) = 0;
    virtual void determine_schedule(unsigned int day_index, unsigned int start_hour, unsigned int end_hour) = 0;
};

class Receptionist : public Staff {
public:
    using Staff::Staff;
    [[nodiscard]] std::string get_profession() const noexcept override;
    void determine_schedule(unsigned int day_index, unsigned int shift_type) override;
    void determine_schedule(unsigned int day_index, unsigned int start_hour, unsigned int end_hour) override;
    void display_info() const override;
    unsigned int calculate_salary() const override;
};

class Maid : public Staff {
public:
    using Staff::Staff;
    [[nodiscard]] std::string get_profession() const noexcept override;
    void determine_schedule(unsigned int day_index, unsigned int shift_type) override;
    void determine_schedule(unsigned int day_index, unsigned int start_hour, unsigned int end_hour) override;
    void display_info() const override;
    unsigned int calculate_salary() const override;
};

class Chef : public Staff {
public:
    using Staff::Staff;
    [[nodiscard]] std::string get_profession() const noexcept override;
    void determine_schedule(unsigned int day_index, unsigned int shift_type) override;
    void determine_schedule(unsigned int day_index, unsigned int start_hour, unsigned int end_hour) override;
    void display_info() const override;
    unsigned int calculate_salary() const override;
};