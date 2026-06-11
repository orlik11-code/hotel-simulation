#include "Guest.hpp"

class Family : public Guest {
public:
  using Guest::Guest;
  virtual void reduce_satisfaction(int default_value) override;
};