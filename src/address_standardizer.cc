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

void parse_delivery_line(const std::vector<token> &tokens, size_t end, delivery_line *dl)
{
  size_t i = 0;
  if (end == 0)
    return;

  if (end - i > 1)
  {
    dl->house_number = tokens[i].text;
    ++i;
  }

  auto direction = lookup(directional, tokens[i].text);
  if (i < end && direction.has_value() && end - i > 1)
  {
    dl->pre_directional = direction;
    ++i;
  }

  size_t unit_at = end;
  for (size_t k = i; k < end; ++k)
  {
    if (k == i)
      continue;

    auto res = lookup(unit, tokens[k].text);
    if (res.has_value())
    {
      unit_at = k;
      break;
    }
  }

  size_t street_end = unit_at;

  if (unit_at < end)
  {
    auto res = lookup(unit, tokens[unit_at].text);
    dl->unit_designator = res.value();

    size_t i = unit_at + 1;
    if (dl->unit_identifier != "#" && i < end && tokens[i].text == "#")
      ++i;

    if (i < end && standalone.count(tokens[i].text) == 0)
    {
      dl->unit_identifier = tokens[i].text;
      i++;
    }

    if (i < end)
    {
      // todo: warn, tokens after secondary address ignored
    }
  }

  auto res = lookup(directional, tokens[street_end - 1].text);
  if (street_end - 1 >= 2 && res.has_value())
  {
    dl->post_directional = res.value();
    --street_end;
  }

  auto sufx = lookup(suffix, tokens[street_end - 1].text);
  if (street_end - 1 >= 2 && sufx.has_value())
  {
    dl->street_suffix = sufx.value();
    --street_end;
  }

  for (size_t k = i; k < street_end; ++k)
  {
    if (!dl->street_name.empty())
      dl->street_name.push_back(' ');

    dl->street_name += tokens[k].text;
  }

  if (dl->street_name.empty() && !dl->street_suffix.empty())
  {
    dl->street_name = dl->street_suffix;
    dl->street_suffix.clear();
    // todo add warning
    // street name is also a suffix word
  }
}

std::string address::to_string()
{
  std::string res;
  res += line1.house_number += " ";

  if (line1.pre_directional.has_value())
    res += line1.pre_directional.value() += " ";

  res += line1.street_name += " ";
  res += line1.street_suffix += " ";

  if (line1.post_directional.has_value())
    res += line1.post_directional.value() + " ";

  if (line1.unit_designator.has_value())
    res += line1.unit_designator.value() + " ";

  if (line1.unit_identifier.has_value())
    res += line1.unit_identifier.value() + " ";

  res.erase(res.length() - 1);
  res += ", ";

  res += line2.city += " ";
  res += line2.state += " ";
  res += line2.zip;

  if (line2.ext.has_value())
    res += "-" + line2.ext.value();

  return res;
}

std::string address::to_json()
{
  std::string res = "{";
  res += R"("house_number":")" + line1.house_number + R"(", )";

  if (line1.pre_directional.has_value())
    res += R"("pre_directional":")" + line1.pre_directional.value() + R"(", )";

  res += R"("street_name":")" + line1.street_name + R"(", )";
  res += R"("street_suffix":")" + line1.street_suffix + R"(", )";

  if (line1.post_directional.has_value())
    res += R"("post_directional":")" + line1.post_directional.value() + R"(", )";

  if (line1.unit_designator.has_value())
    res += R"("unit_designator":")" + line1.unit_designator.value() + R"(", )";

  if (line1.unit_identifier.has_value())
    res += R"("unit_identifier":")" + line1.unit_identifier.value() + R"(", )";

  res += line2.city += " ";
  res += line2.state += " ";
  res += line2.zip;

  res += R"("city":")" + line2.city + R"(", )";
  res += R"("state":")" + line2.state + R"(", )";
  res += R"("zip":")" + line2.zip;

  if (line2.ext.has_value())
    res += "-" + line2.ext.value();

  res += R"(")";
  res += "}";

  return res;
}
} // namespace vsql_addr_std
