#include <gtest/gtest.h>
#include <iostream>
#include <vector>

#include "address_standardizer.h"

using vsql_addr_std::token;

TEST(address_standardizer, tokenizer)
{
  std::string addr1 = "123 main st apt 4, springfield il 62704";
  auto res = vsql_addr_std::tokenise(addr1);
  std::vector<token> expected{
      token{"123", false}, token{"MAIN", false},        token{"ST", false}, token{"APT", false},
      token{"4", true},    token{"SPRINGFIELD", false}, token{"IL", false}, token{"62704", false},
  };

  ASSERT_EQ(res, expected);
}
