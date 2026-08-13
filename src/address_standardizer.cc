#include <algorithm>
#include <cctype>
#include <cstddef>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "address_standardizer.h"
#include "us.h"

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

bool all_digits(const std::string &s)
{
  return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c); });
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

std::vector<token> tokenise(std::string_view address)
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
  }

  flush();
  return res;
}

// valid zips 12345, 12345-1234, 123456789
bool parse_zip(const std::string &t, std::string *zip, std::optional<std::string> *ext)
{
  if (t.size() == 5 && all_digits(t))
  {
    *zip = t;
    if (ext->has_value())
      ext->value().clear();
    return true;
  }

  if (t.size() == 10 && t[5] == '-' && all_digits(t.substr(0, 5)))
  {
    *zip = t.substr(0, 5);
    *ext = t.substr(6);
    return true;
  }

  if (t.size() == 9 && all_digits(t))
  {
    *zip = t.substr(0, 5);
    *ext = t.substr(5);
    return true;
  }

  return false;
}

size_t parse_last_line(const std::vector<token> &tokens, last_line *ll)
{
  auto end = tokens.size();
  if (end == 0)
    return 0;

  if (parse_zip(tokens[end - 1].text, &ll->zip, &ll->ext))
  {
    --end;
  }
  if (end == 0)
    return 0;

  // state
  for (size_t words = 4; words >= 1; --words)
  {
    if (end < words)
      continue;
    std::string joined;

    for (size_t i = end - words; i < end; ++i)
    {
      if (!joined.empty())
        joined.push_back(' ');

      joined += tokens[i].text;
    }

    auto code = lookup(states, joined);
    if (code.has_value())
    {
      ll->state = code.value();
      end -= words;
      break;
    }

    if (words == 1 && state_codes().count(joined))
    {
      ll->state = joined;
      end -= words;
      break;
    }
  }

  if (end == 0)
    return 0;

  // city
  size_t city_start = end;
  for (size_t i = end; i > 0; i--)
  {
    if (tokens[i - 1].comma_after)
    {
      city_start = i;
      break;
    }
  }

  // TODO: handle the case where there's no comma before city

  for (size_t i = city_start; i < end; i++)
  {
    if (!ll->city.empty())
      ll->city.push_back(' ');

    ll->city += tokens[i].text;
  }

  return city_start;
}
} // namespace vsql_addr_std
