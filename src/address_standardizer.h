#include <optional>
#include <string>

struct address {
  std::optional<std::string> house_number;
  std::optional<std::string> street_name;
  std::optional<std::string> street_suffix;

  std::optional<std::string> city;
  std::optional<std::string> state;
  std::optional<std::string> zip;
};
