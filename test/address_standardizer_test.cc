#include <gtest/gtest.h>
#include <iostream>

#include "address_standardizer.h"

TEST(address_standardizer, basic_address)
{
  address_standardizer parser{};
  auto address = parser.parse("address");

  std::cout << "testing a basic address";
}
