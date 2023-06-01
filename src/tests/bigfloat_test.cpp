#include <gtest/gtest.h>

#include "komori/bigfloat.hpp"

using komori::BigFloat;
using komori::BigUint;

TEST(BigFloat, Add) {
  BigFloat zero(1000);
  BigFloat x(10, BigUint({0b1010101011}), 2);
  BigFloat y(5, BigUint({0b11111}), 0);

  EXPECT_EQ((x + y).IntegerPart(), BigUint({(0b1010101011 << 2) + 0b11111}));
}

TEST(BigFloat, IntegerPart) {
  BigFloat x(334);
  BigFloat y(334, BigUint({0x334}), 20);
  BigFloat z(334, BigUint({0x334}), -2);
  BigFloat w(334, BigUint({0x334}), -100);

  EXPECT_EQ(x.IntegerPart(), BigUint{});
  EXPECT_EQ(y.IntegerPart(), BigUint({0x334 << 20}));
  EXPECT_EQ(z.IntegerPart(), BigUint({0x334 >> 2}));
  EXPECT_EQ(w.IntegerPart(), BigUint{});
}

TEST(BigFloat, FractionalPart) {
  struct TestCase {
    BigFloat input;
    BigUint expected_first;
    int64_t expected_second;
  };

  const std::array<TestCase, 4> test_cases = {{
      // zero
      {BigFloat(334), BigUint{}, 0},
      // Integer
      {BigFloat(334, BigUint({0x334}), 20), BigUint{}, 0},
      // 0x3.34
      {BigFloat(334, BigUint({0x334}), -8), BigUint({0x34}), -8},
      // 0x0.00 .. 0334
      {BigFloat(334, BigUint({0x334}), -100), BigUint({0x334}), -100},
  }};

  for (const auto& [input, expected_first, expected_second] : test_cases) {
    const auto res = input.FractionalPart();
    EXPECT_EQ(res.first, expected_first);
    EXPECT_EQ(res.second, expected_second);
  }
}