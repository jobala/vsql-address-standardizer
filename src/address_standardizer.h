#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace vsql_addr_std
{
struct delivery_line
{
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
  std::string city;
  std::string state;
  std::string zip;
};

struct address
{
  delivery_line line1;
  last_line line2;
};

struct address_standardizer
{
  address parse(std::string_view place);
};
} // namespace vsql_addr_std
