#include <gtest/gtest.h>

#include "bigfloat.hpp"

using komori::BigFloat;
using komori::BigUint;
using komori::Sign;

TEST(BigFloat, SignAndUnaryMinus) {
  const BigFloat x(10);
  const BigFloat y(10, BigUint{0x334});

  EXPECT_EQ(x.GetSign(), Sign::kPositive);
  EXPECT_EQ(y.GetSign(), Sign::kPositive);
  EXPECT_EQ((-x).GetSign(), Sign::kNegative);
  EXPECT_EQ((-y).GetSign(), Sign::kNegative);
}

TEST(BigFloat, DebugString) {
  EXPECT_EQ(BigFloat(128, BigUint{}).DebugString(), "0x0 * 2^0");
  EXPECT_EQ(((-BigFloat(128, BigUint{0x334})) >> 334).DebugString(), "-0x334 * 2^(-334)");
}

TEST(BigFloat, Add) {
  struct TestCase {
    BigFloat lhs;
    BigFloat rhs;
    Sign expected_sign;
    BigUint expected_significand;
    int64_t expected_fractional_digits;
    int64_t expected_precision;
  };

  const BigFloat x = BigFloat(20, BigUint{0x334});
  const BigFloat y = BigFloat(10, BigUint{0x264});
  const std::array<TestCase, 6> test_cases{{
      {x << 33, y, Sign::kPositive, BigUint{(0x334ULL << 33) + 0x264}, 0, 20},
      {x >> 4, y, Sign::kPositive, BigUint{0x334ULL + (0x264 << 4)}, 4, 10},
      {x, y, Sign::kPositive, BigUint{0x334 + 0x264}, 0, 11},
      {x, -y, Sign::kPositive, BigUint{0x334 - 0x264}, 0, 8},
      {-x, y, Sign::kNegative, BigUint{0x334 - 0x264}, 0, 8},
      {-x, -y, Sign::kNegative, BigUint{0x334 + 0x264}, 0, 11},
  }};

  for (const auto& [lhs, rhs, e_sign, e_significand, e_fractional_digits, e_precision] : test_cases) {
    const auto ans = lhs + rhs;
    const auto sign = ans.GetSign();
    const auto precision = ans.GetPrecision();
    const auto integer_part = (ans << e_fractional_digits).IntegerPart();

    EXPECT_EQ(sign, e_sign);
    EXPECT_EQ(precision, e_precision);
    EXPECT_EQ(integer_part, e_significand);
  }
}

TEST(BigFloat, Multiply) {
  const BigFloat x = BigFloat(128, BigUint{0x334ULL, 0x264ULL}) << 33;
  const BigFloat y = BigFloat(16, BigUint{0x44ULL, 0x5ULL}) >> 4;

  const auto z = x * y;
  EXPECT_EQ(z.GetSign(), Sign::kPositive);

  // (0x264 * 0x5) == 0xbf4
  EXPECT_EQ((z >> (128 + 29)).IntegerPart(), BigUint{0xbf4});

  EXPECT_EQ((x * (-y)).GetSign(), Sign::kNegative);
}

TEST(BigFloat, IntegerPart) {
  BigFloat x(334);
  BigFloat y = BigFloat(334, BigUint{0x334}) << 20;
  BigFloat z = BigFloat(334, BigUint{0x334}) >> 2;
  BigFloat w = BigFloat(334, BigUint{0x334}) >> 100;

  EXPECT_EQ(x.IntegerPart(), BigUint{});
  EXPECT_EQ(y.IntegerPart(), BigUint{0x334 << 20});
  EXPECT_EQ(z.IntegerPart(), BigUint{0x334 >> 2});
  EXPECT_EQ(w.IntegerPart(), BigUint{});
}

TEST(BigFloat, FractionalPart) {
  struct TestCase {
    BigFloat input;
    int64_t expected_precision;
    BigUint expected_significand;
    int64_t expected_exp;
  };

  const std::array<TestCase, 4> test_cases = {{
      // zero
      {BigFloat(334), 334, BigUint{}, -334},
      // Integer
      {BigFloat(334, BigUint{0x334}) << 20, 334 - 20 - 10, BigUint{}, 0},
      // 0x3.34
      {BigFloat(334, BigUint{0x334}) >> 8, 334 - 2, BigUint{0x34}, -8},
      // 0x0.00 .. 0334
      {BigFloat(334, BigUint{0x334}) >> 100, 334 + 100 - 10, BigUint{0x334}, -100},
  }};

  for (const auto& [input, e_precision, e_significand, e_exp] : test_cases) {
    const auto res = input.FractionalPart();
    EXPECT_EQ(res.GetPrecision(), e_precision);

    const auto sig = (res << (-e_exp)).IntegerPart();
    EXPECT_EQ(sig, e_significand);
  }
}

TEST(BigFloat, ApproximateFloat) {
  EXPECT_THROW((BigFloat(128) << 334).ApproximateInverse(), std::range_error);

  BigFloat x = BigFloat(128, BigUint{0x1}) << 334;
  BigFloat inv_x = x.ApproximateInverse() << 334;
  EXPECT_EQ(inv_x.GetSign(), Sign::kPositive);
  EXPECT_EQ(inv_x.IntegerPart(), BigUint{1});

  BigFloat y = (-BigFloat(128, BigUint{0x334})) >> 334;
  BigFloat inv_y = y.ApproximateInverse() >> (334 - 64);
  EXPECT_EQ(inv_y.GetSign(), Sign::kNegative);
  EXPECT_EQ(inv_y.IntegerPart(), BigUint{0x4FEC04FEC04FEC});
}

TEST(BigFloat, Inverse) {
  BigFloat x = BigFloat(128, BigUint{1}) << 334;
  auto inv_x = Inverse(x) << 334;
  EXPECT_EQ(inv_x.IntegerPart(), BigUint{1});

  BigFloat y = (-BigFloat(128, BigUint{0x123456789ABCDEF0ULL, 0xFEDCBA0987654321ULL})) >> 334;
  BigFloat inv_y = Inverse(y);
  BigFloat prod = y * inv_y;
  BigUint frac_part = (prod << 128).IntegerPart();
  EXPECT_TRUE(frac_part == (BigUint{0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL}) || frac_part == (BigUint{0, 0, 1}));
}

TEST(BigFloat, Sqrt) {
  BigFloat x = BigFloat(128, BigUint{0x123456789ABCDEF0ULL, 0xFEDCBA0987654321ULL}) >> 334;
  BigFloat sqrt_x = Sqrt(x);
  BigUint y = ((sqrt_x * sqrt_x) << 334).IntegerPart();
  EXPECT_TRUE(y == (BigUint{0x123456789ABCDEF0ULL, 0xFEDCBA0987654321ULL}) ||
              y == (BigUint{0x123456789ABCDEEFULL, 0xFEDCBA0987654321ULL}))
      << y.DebugString();
}