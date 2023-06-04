#include <gtest/gtest.h>

#include "komori/io.hpp"

using komori::BigFloat;
using komori::BigUint;

TEST(OutputOperator, BigFloat) {
  BigFloat x = BigFloat(128, BigUint{334334334334ULL}) / BigFloat(128, BigUint{1000000000000ULL});

  const auto s = ToString(x).substr(0, 20);
  EXPECT_TRUE(s == "0.334334334334000000" || s == "0.334334334333999999");
}