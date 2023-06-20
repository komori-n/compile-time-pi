#include <gtest/gtest.h>

#include "komori/common.hpp"

using komori::Sign;

TEST(Common, SignOperators) {
  const auto p = Sign::kPositive;
  const auto n = Sign::kNegative;

  EXPECT_EQ(~p, n);
  EXPECT_EQ(~n, p);
  EXPECT_EQ(p ^ p, p);
  EXPECT_EQ(p ^ n, n);
  EXPECT_EQ(n ^ p, n);
  EXPECT_EQ(n ^ n, p);
}
