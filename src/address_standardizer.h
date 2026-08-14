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

  bool operator==(const delivery_line &other) const
  {
    return house_number == other.house_number && street_name == other.street_name &&
           street_suffix == other.street_suffix && pre_directional == other.pre_directional &&
           post_directional == other.post_directional && unit_identifier == other.unit_identifier &&
           unit_designator == other.unit_designator;
  }

  std::string house_number;
  std::string street_name;
  std::string street_suffix;

  std::optional<std::string> pre_directional;
  std::optional<std::string> post_directional;
  std::optional<std::string> unit_designator;
  std::optional<std::string> unit_identifier;
};

struct last_line
{
  bool operator==(const last_line &other) const
  {
    return city == other.city && state == other.state && zip == other.zip && ext == other.ext;
  }

  std::string city;
  std::string state;
  std::string zip;

  std::optional<std::string> ext;
};

struct address
{
  delivery_line line1;
  last_line line2;
};

struct token
{
  bool operator==(const token &other) const { return text == other.text && comma_after == other.comma_after; }

  const std::string text;
  bool comma_after;
};

std::vector<token> tokenise(std::string_view address);
size_t parse_last_line(const std::vector<token> &tokens, last_line *ll);
void parse_delivery_line(const std::vector<token> &tokens, size_t end, delivery_line *dl);

// used by gtest for string output
inline void PrintTo(const token &t, std::ostream *os)
{
  *os << "token{text: \"" << t.text << "\", comma_after: " << t.comma_after << "}";
}

inline void PrintTo(const last_line &ll, std::ostream *os)
{
  *os << "last_line{city: \"" << ll.city << "\", state: " << ll.state << "\", zip: " << ll.zip << "\", ext: " << *ll.ext
      << "}";
}

inline void PrintTo(const delivery_line &dl, std::ostream *os)
{
  *os << "delivery_line{house_number: \"" << dl.house_number << "\", street_name: " << dl.street_name
      << "\", street_suffix: " << dl.street_suffix << "\", pre_directional: " << *dl.pre_directional
      << "\", post_directional: " << *dl.post_directional << "\", unit_designator: " << *dl.unit_designator
      << "\", unit_identifier: " << *dl.unit_identifier << "}";
}
} // namespace vsql_addr_std
