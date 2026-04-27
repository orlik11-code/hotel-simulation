//
// Created by orlik on 27.04.2026.
//

#ifndef PROI_ZAD4_ROOM_H
#define PROI_ZAD4_ROOM_H

class Room{
public:
    unsigned int get_price() const;
    unsigned int get_workers_needed() const;
    unsigned int get_single_beds() const;
    unsigned int get_double_beds() const;
    unsigned int get_number() const;
    unsigned int get_floor() const;
    double get_area() const;

    bool is_to_clean() const;
    bool is_to_repair() const;

    void set_price(unsigned int price);
    void set_workers_needed(unsigned int workers_needed);
    void set_number(unsigned int number);
    void set_area(double area);

    void clean();
    void make_dirty();
    void repair();
    void destroy();
    void new_config(unsigned int s_beds, unsigned int d_beds);

private:
    double area;
    unsigned int price;
    unsigned int workers_needed;
    unsigned int number;
    unsigned int floor;
    unsigned int single_beds;
    unsigned int double_beds;
    bool to_clean;
    bool to_repair;

};
#endif //PROI_ZAD4_ROOM_H
