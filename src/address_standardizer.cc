#pragma once

#include "address_standardizer.h"
#include "us.h"
#include <cctype>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace vsql_addr_std
{
std::unordered_set<std::string> state_codes()
{
  std::unordered_set<std::string> res{};
  for (const auto &entry : states)
  {
    res.insert(entry.second);
  }

  return res;
}

std::optional<std::string> lookup(const std::unordered_map<std::string, std::string> &table, const std::string &key)
{
  auto iter = table.find(key);
  if (iter == table.end())
  {
    return std::nullopt;
  }
  return iter->second;
}

std::vector<token> tokenize(std::string_view address)
{
  std::vector<token> res;
  std::string curr;
  bool pending_comma = false;

  auto flush = [&] {
    if (!curr.empty())
    {
      res.push_back({curr, false});
      curr.clear();
    }
  };

  for (char raw : address)
  {
    unsigned char c = static_cast<unsigned char>(raw);
    if (c == ',')
    {
      flush();
      if (!res.empty())
      {
        res.back().comma_after = true;
        continue;
      }
    }

    if (std::isspace(c))
    {
      flush();
      continue;
    }

    if (c == '#')
    {
      flush();
      res.push_back({"#", false});
      continue;
    }

    if (std::isalnum(c))
    {
      curr.push_back(static_cast<char>(std::toupper(c)));
      continue;
    }

    if (c == '-' || c == '/' || c == '&')
    {
      if (!curr.empty())
      {
        curr.push_back(static_cast<char>(c));
        continue;
      }
    }
    flush();

    return res
  }

  return res;
}
} // namespace vsql_addr_std
