#include "Guest.hpp"

class Family : public Guest {
public:
  using Guest::Guest;
  void virtual reduce_satisfaction(int default_value) override;
};