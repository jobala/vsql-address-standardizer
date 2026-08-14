#include <gtest/gtest.h>
#include <vector>

#include "address_standardizer.h"

using vsql_addr_std::delivery_line;
using vsql_addr_std::last_line;
using vsql_addr_std::parse_delivery_line;
using vsql_addr_std::parse_last_line;
using vsql_addr_std::token;
using vsql_addr_std::tokenise;

TEST(address_standardizer, tokenizer)
{
  std::string addr1 = "123 main st apt 4, springfield il 62704";
  auto res = tokenise(addr1);
  std::vector<token> expected{
      token{"123", false}, token{"MAIN", false},        token{"ST", false}, token{"APT", false},
      token{"4", true},    token{"SPRINGFIELD", false}, token{"IL", false}, token{"62704", false},
  };

  ASSERT_EQ(res, expected);
}

TEST(address_standardizer, parse_last_line)
{
  std::string addr1 = "123 main st apt 4, springfield illinois 62704";
  vsql_addr_std::last_line ll;

  auto tokens = tokenise(addr1);
  parse_last_line(tokens, &ll);
  last_line expected{.city = "SPRINGFIELD", .state = "IL", .zip = "62704"};

  ASSERT_EQ(ll, expected);
}

TEST(address_standardizer, parse_last_line_with_zip_ext)
{
  std::string addr1 = "123 main st apt 4, springfield illinois 62704-1234";
  vsql_addr_std::last_line ll;

  auto tokens = tokenise(addr1);
  parse_last_line(tokens, &ll);
  last_line expected{.city = "SPRINGFIELD", .state = "IL", .zip = "62704", .ext = "1234"};

  ASSERT_EQ(ll, expected);
}

TEST(address_standardizer, parse_last_line_with_multiword_state)
{
  std::string addr1 = "123 main st apt 4, springfield new york 62704-1234";
  vsql_addr_std::last_line ll;

  auto tokens = tokenise(addr1);
  parse_last_line(tokens, &ll);
  last_line expected{.city = "SPRINGFIELD", .state = "NY", .zip = "62704", .ext = "1234"};

  ASSERT_EQ(ll, expected);
}

TEST(address_standardizer, delivery_line)
{
  std::string addr1 = "123 north main burlington st south apt 4 , springfield new york 62704-1234";
  last_line ll;
  delivery_line dl;

  auto tokens = tokenise(addr1);
  auto delivery_line_end = parse_last_line(tokens, &ll);
  parse_delivery_line(tokens, delivery_line_end, &dl);

  delivery_line expected{.house_number = "123",
                         .street_name = "MAIN",
                         .street_suffix = "ST",
                         .pre_directional = "N",
                         .post_directional = "S",
                         .secondary_address_id = "APT",
                         .secondary_address = "4"};

  ASSERT_EQ(dl, expected);
}
