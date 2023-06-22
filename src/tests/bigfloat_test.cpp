#include <gtest/gtest.h>

#include "bigfloat.hpp"

using komori::BigFloat;
using komori::BigInt;
using komori::BigUint;
using komori::Sign;

TEST(BigFloat, DebugString) {
  EXPECT_EQ(BigFloat(128, BigInt{}).DebugString(), "+0x0 * 2^0");
  EXPECT_EQ(((-BigFloat(128, BigInt{0x334})) >> 334).DebugString(), "-0x334 * 2^(-334)");
}

TEST(BigFloat, Add) {
  struct TestCase {
    BigFloat lhs;
    BigFloat rhs;
    BigInt expected_significand;
    int64_t expected_fractional_digits;
    int64_t expected_precision;
  };

  const BigFloat x = BigFloat(20, BigInt{0x334});
  const BigFloat y = BigFloat(10, BigInt{0x264});
  const std::array<TestCase, 6> test_cases{{
      {x << 33, y, BigInt{(0x334ULL << 33) + 0x264}, 0, 20},
      {x >> 4, y, BigInt{0x334ULL + (0x264 << 4)}, 4, 10},
      {x, y, BigInt{0x334 + 0x264}, 0, 11},
      {x, -y, BigInt{0x334 - 0x264}, 0, 8},
      {-x, y, BigInt{0x334 - 0x264, Sign::kNegative}, 0, 8},
      {-x, -y, BigInt{0x334 + 0x264, Sign::kNegative}, 0, 11},
  }};

  for (const auto& [lhs, rhs, e_significand, e_fractional_digits, e_precision] : test_cases) {
    const auto ans = lhs + rhs;
    const auto precision = ans.GetPrecision();
    const auto integer_part = (ans << e_fractional_digits).IntegerPart();

    EXPECT_EQ(precision, e_precision);
    EXPECT_EQ(integer_part, e_significand);
  }
}

TEST(BigFloat, Multiply) {
  const BigFloat x = BigFloat(128, BigInt{0x334ULL, 0x264ULL}) << 33;
  const BigFloat y = BigFloat(16, BigInt{0x44ULL, 0x5ULL}) >> 4;

  const auto z = x * (-y);

  // (0x264 * 0x5) == 0xbf4
  EXPECT_EQ((z >> (128 + 29)).IntegerPart(), BigInt(0xbf4, Sign::kNegative));
}

TEST(BigFloat, IntegerPart) {
  BigFloat x(334);
  BigFloat y = BigFloat(334, BigInt{0x334}) << 20;
  BigFloat z = BigFloat(334, BigInt{0x334}) >> 2;
  BigFloat w = BigFloat(334, BigInt{0x334}) >> 100;

  EXPECT_EQ(x.IntegerPart(), BigInt{});
  EXPECT_EQ(y.IntegerPart(), BigInt{0x334 << 20});
  EXPECT_EQ(z.IntegerPart(), BigInt{0x334 >> 2});
  EXPECT_EQ(w.IntegerPart(), BigInt{});
}

TEST(BigFloat, FractionalPart) {
  struct TestCase {
    BigFloat input;
    int64_t expected_precision;
    BigInt expected_significand;
    int64_t expected_exp;
  };

  const std::array<TestCase, 4> test_cases = {{
      // zero
      {BigFloat(334), 334, BigInt{}, -334},
      // Integer
      {BigFloat(334, BigInt{0x334}) << 20, 334 - 20 - 10, BigInt{}, 0},
      // 0x3.34
      {BigFloat(334, BigInt{0x334}) >> 8, 334 - 2, BigInt{0x34}, -8},
      // 0x0.00 .. 0334
      {BigFloat(334, BigInt{0x334}) >> 100, 334 + 100 - 10, BigInt{0x334}, -100},
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

  BigFloat x = BigFloat(128, BigInt{0x1}) << 334;
  BigFloat inv_x = x.ApproximateInverse() << 334;
  EXPECT_EQ(inv_x.IntegerPart(), BigInt{1});

  BigFloat y = (-BigFloat(128, BigInt{0x334})) >> 334;
  BigFloat inv_y = y.ApproximateInverse() >> (334 - 64);
  EXPECT_EQ(inv_y.IntegerPart(), BigInt(0x4FEC04FEC04FEC, Sign::kNegative));
}

TEST(BigFloat, Inverse) {
  BigFloat x = BigFloat(128, BigInt{1}) << 334;
  auto inv_x = Inverse(x) << 334;
  EXPECT_EQ(inv_x.IntegerPart(), BigInt{1});

  BigFloat y = (-BigFloat(128, BigInt{0x123456789ABCDEF0ULL, 0xFEDCBA0987654321ULL})) >> 334;
  BigFloat inv_y = Inverse(y);
  BigFloat prod = y * inv_y;
  BigInt frac_part = (prod << 128).IntegerPart();
  EXPECT_TRUE(frac_part == (BigInt{0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL}) || frac_part == (BigInt{0, 0, 1}));
}

TEST(BigFloat, Sqrt) {
  BigFloat x = BigFloat(128, BigInt{0x123456789ABCDEF0ULL, 0xFEDCBA0987654321ULL}) >> 334;
  BigFloat sqrt_x = Sqrt(x);
  BigInt y = ((sqrt_x * sqrt_x) << 334).IntegerPart();
  EXPECT_TRUE(y == (BigInt{0x123456789ABCDEF0ULL, 0xFEDCBA0987654321ULL}) ||
              y == (BigInt{0x123456789ABCDEEFULL, 0xFEDCBA0987654321ULL}))
      << y.DebugString();
}