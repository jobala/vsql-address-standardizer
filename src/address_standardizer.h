#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace vsql_addr_std
{
struct delivery_line
{
  bool empty();

  std::string house_number;
  std::string street_name;
  std::string street_suffix;

  std::optional<std::string> pre_directional;
  std::optional<std::string> post_directional;
  std::optional<std::string> secondary_address_id;
  std::optional<std::string> secondary_address;
};

struct last_line
{
  bool empty();

  std::string city;
  std::string state;
  std::string zip;
};

struct address
{
  bool empty();

  delivery_line line1;
  last_line line2;
};

struct token
{
  bool operator==(const token &other) const { return text == other.text && comma_after == other.comma_after; }

  std::string text;
  bool comma_after;
};

std::vector<token> tokenise(std::string_view address);

// used by gtest for string output
inline void PrintTo(const token &t, std::ostream *os)
{
  *os << "token{text: \"" << t.text << "\", comma_after: " << t.comma_after << "}";
}
} // namespace vsql_addr_std
