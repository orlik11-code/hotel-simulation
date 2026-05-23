#include <string>

class Guest {
public:
  Guest(std::string name, std::string surname);

  std::string get_name();
  std::string get_surname();
  int get_satisfaction();

  void virtual reduce_satisfaction(int default_value) = 0;
  virtual ~Guest() = default;

protected:
  std::string name;
  std::string surname;
  int satisfaction = 100;
};