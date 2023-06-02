#include <gtest/gtest.h>

#include "komori/bigfloat.hpp"

using komori::BigFloat;
using komori::BigUint;

TEST(BigFloat, Sign) {
  const BigFloat x(10);
  const BigFloat y(10, BigUint({0x334}));

  EXPECT_EQ(x.GetSign(), BigFloat::Sign::kPositive);
  EXPECT_EQ(y.GetSign(), BigFloat::Sign::kPositive);
  EXPECT_EQ((-x).GetSign(), BigFloat::Sign::kNegative);
  EXPECT_EQ((-y).GetSign(), BigFloat::Sign::kNegative);
}

TEST(BigFloat, Add) {
  using Sign = BigFloat::Sign;
  struct TestCase {
    BigFloat lhs;
    BigFloat rhs;
    Sign expected_sign;
    BigUint expected_integer_part;
    BigUint expected_fractional_part;
    int64_t expected_fractional_digits;
    int64_t expected_precision;
  };

  const BigFloat x = BigFloat(20, BigUint({0x334}));
  const BigFloat y = BigFloat(10, BigUint({0x264}));
  const std::array<TestCase, 6> test_cases{{
      {x << 33, y, Sign::kPositive, BigUint({(0x334ULL << 33) + 0x264}), BigUint{}, 0, 20},
      {x >> 4, y, Sign::kPositive, BigUint({(0x334ULL >> 4) + 0x264}), BigUint{0x4}, -4, 10},
      {x, y, Sign::kPositive, BigUint({0x334 + 0x264}), BigUint{}, 0, 11},
      {x, -y, Sign::kPositive, BigUint({0x334 - 0x264}), BigUint{}, 0, 8},
      {-x, y, Sign::kNegative, BigUint({0x334 - 0x264}), BigUint{}, 0, 8},
      {-x, -y, Sign::kNegative, BigUint({0x334 + 0x264}), BigUint{}, 0, 11},
  }};

  for (const auto& [lhs, rhs, e_sign, e_integer_part, e_fractional_part, e_fractional_digits, e_precision] :
       test_cases) {
    const auto ans = lhs + rhs;
    const auto sign = ans.GetSign();
    const auto precision = ans.GetPrecision();
    const auto integer_part = ans.IntegerPart();
    const auto [fractional_part, fractional_digits] = ans.FractionalPart();

    EXPECT_EQ(sign, e_sign);
    EXPECT_EQ(precision, e_precision);
    EXPECT_EQ(integer_part, e_integer_part);
    EXPECT_EQ(fractional_part, e_fractional_part);
    EXPECT_EQ(fractional_digits, e_fractional_digits);
  }
}

TEST(BigFloat, IntegerPart) {
  BigFloat x(334);
  BigFloat y = BigFloat(334, BigUint({0x334})) << 20;
  BigFloat z = BigFloat(334, BigUint({0x334})) >> 2;
  BigFloat w = BigFloat(334, BigUint({0x334})) >> 100;

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
      {BigFloat(334, BigUint({0x334})) << 20, BigUint{}, 0},
      // 0x3.34
      {BigFloat(334, BigUint({0x334})) >> 8, BigUint({0x34}), -8},
      // 0x0.00 .. 0334
      {BigFloat(334, BigUint({0x334})) >> 100, BigUint({0x334}), -100},
  }};

  for (const auto& [input, expected_first, expected_second] : test_cases) {
    const auto res = input.FractionalPart();
    EXPECT_EQ(res.first, expected_first);
    EXPECT_EQ(res.second, expected_second);
  }
}