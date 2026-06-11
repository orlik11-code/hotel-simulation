#include "Guest.hpp"

class Business : public Guest {
public:
  using Guest::Guest;
  virtual void reduce_satisfaction(int default_value) override;
};