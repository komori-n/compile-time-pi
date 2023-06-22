#include <gtest/gtest.h>

#include "bigint.hpp"

using komori::BigInt;
using komori::Sign;

TEST(BigInt, IsZero) {
  EXPECT_TRUE(BigInt{}.IsZero());
  EXPECT_FALSE(BigInt(0x334ULL).IsZero());
  EXPECT_FALSE(BigInt(0x334ULL, Sign::kNegative).IsZero());
}

TEST(BigInt, Add) {
  const BigInt x(0x334ULL);
  const BigInt y(0x264ULL);

  EXPECT_EQ(x + BigInt{}, x);
  EXPECT_EQ(y + BigInt{}, y);
  EXPECT_EQ((-x) + BigInt{}, -x);
  EXPECT_EQ((-y) + BigInt{}, -y);

  EXPECT_EQ(x + y, BigInt(0x334ULL + 0x264ULL));
  EXPECT_EQ(x + (-y), BigInt(0x334ULL - 0x264ULL));
  EXPECT_EQ((-x) + y, BigInt(0x334ULL - 0x264ULL, Sign::kNegative));
  EXPECT_EQ((-x) + (-y), BigInt(0x334ULL + 0x264ULL, Sign::kNegative));

  EXPECT_EQ(y + x, BigInt(0x334ULL + 0x264ULL));
  EXPECT_EQ(y + (-x), BigInt(0x334ULL - 0x264ULL, Sign::kNegative));
  EXPECT_EQ((-y) + x, BigInt(0x334ULL - 0x264ULL));
  EXPECT_EQ((-y) + (-x), BigInt(0x334ULL + 0x264ULL, Sign::kNegative));
}

TEST(BigInt, Sub) {
  const BigInt x(0x334ULL);
  const BigInt y(0x264ULL);

  EXPECT_EQ(x + BigInt{}, x);
  EXPECT_EQ(y + BigInt{}, y);
  EXPECT_EQ((-x) + BigInt{}, -x);
  EXPECT_EQ((-y) + BigInt{}, -y);

  EXPECT_EQ(x - y, BigInt(0x334ULL - 0x264ULL));
  EXPECT_EQ(x - (-y), BigInt(0x334ULL + 0x264ULL));
  EXPECT_EQ((-x) - y, BigInt(0x334ULL + 0x264ULL, Sign::kNegative));
  EXPECT_EQ((-x) - (-y), BigInt(0x334ULL - 0x264ULL, Sign::kNegative));

  EXPECT_EQ(y - x, BigInt(0x334ULL - 0x264ULL, Sign::kNegative));
  EXPECT_EQ(y - (-x), BigInt(0x334ULL + 0x264ULL));
  EXPECT_EQ((-y) - x, BigInt(0x334ULL + 0x264ULL, Sign::kNegative));
  EXPECT_EQ((-y) - (-x), BigInt(0x334ULL - 0x264ULL));
}

TEST(BigInt, Mul) {
  const BigInt x(0x334ULL);
  const BigInt y(0x264ULL);

  EXPECT_EQ(x * y, BigInt(0x334ULL * 0x264ULL));
  EXPECT_EQ(x * (-y), BigInt(0x334ULL * 0x264ULL, Sign::kNegative));
  EXPECT_EQ((-x) * y, BigInt(0x334ULL * 0x264ULL, Sign::kNegative));
  EXPECT_EQ((-x) * (-y), BigInt(0x334ULL * 0x264ULL));
}

TEST(BigInt, Shl) {
  const BigInt x(0x334ULL);
  const BigInt expected(0x3340ULL);

  EXPECT_EQ(x << 4, expected);
  EXPECT_EQ((-x) << 4, -expected);
}

TEST(BigInt, Shr) {
  const BigInt x(0x334ULL);
  const BigInt expected(0x33ULL);

  EXPECT_EQ(x >> 4, expected);
  EXPECT_EQ((-x) >> 4, -expected);
}

TEST(BigInt, Comparison) {
  const BigInt x(0x334ULL);
  const BigInt y(0x264ULL);

  EXPECT_TRUE(x > y);
  EXPECT_TRUE(x > (-y));
  EXPECT_TRUE((-x) < y);
  EXPECT_TRUE((-x) < (-y));
}