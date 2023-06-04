#include <gtest/gtest.h>

#include "komori/io.hpp"

using komori::BigFloat;
using komori::BigUint;

TEST(OutputOperator, BigUint) {
  const auto x = BigUint{0x38c497e5596ef57eULL, 0x4da120763f11e267ULL, 0xefdf8ULL};
  const auto y = BigUint{0xb6d5a4843a6d2a48ULL, 0xdfd9cd030565836eULL, 0xbd99aULL};
  const auto z = BigUint{0x150064843a6d2a48ULL, 0xd3af3f1dfd491f5dULL, 0xd170a19f12b42b61ULL, 0x9c72190cbe98bcc3ULL,
                         0x2a0448625aaULL};

  EXPECT_EQ(ToString(BigUint{}), "0");
  EXPECT_EQ(ToString(x), "334334334334334334334334334334334334334334334");
  EXPECT_EQ(ToString(y), "264264264264264264264264264264264264264264264");
  EXPECT_EQ(ToString(z),
            "334334334334334334334334334334334334334334334"
            "264264264264264264264264264264264264264264264");
}

TEST(OutputOperator, BigFloat) {
  BigFloat x = BigFloat(128, BigUint{334334334334ULL}) / BigFloat(128, BigUint{1000000000ULL});

  const auto s = ToString(x).substr(0, 20);
  EXPECT_TRUE(s == "334.3343343340000000" || s == "334.3343343339999999");
}