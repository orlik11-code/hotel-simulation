#include "Guest.hpp"

class Business : public Guest {
public:
  using Guest::Guest;
  void virtual reduce_satisfaction(int default_value) override;
};