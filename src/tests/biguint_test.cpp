#include <gtest/gtest.h>

#include "komori/biguint.hpp"

using komori::BigUint;

TEST(BigUint, IsZero) {
  EXPECT_TRUE(BigUint{}.IsZero());
  EXPECT_FALSE(BigUint{0x334ULL}.IsZero());
  EXPECT_FALSE((BigUint{0x334ULL, 0x264}).IsZero());
}

TEST(BigUint, NumberOfBits) {
  EXPECT_EQ(BigUint{}.NumberOfBits(), 0);
  EXPECT_EQ(BigUint{0x1}.NumberOfBits(), 1);
  EXPECT_EQ(BigUint{0x334}.NumberOfBits(), 10);
  EXPECT_EQ((BigUint{0x0, 0x1}).NumberOfBits(), 65);
}

TEST(BigUint, Add) {
  const BigUint x{0x8000000000000000ULL, 0x1ULL};
  const BigUint y{0x8000000000000000ULL, 0xFFFFFFFFFFFFFFFEULL};
  const BigUint z{0x334ULL};

  EXPECT_EQ(BigUint{} + BigUint{}, BigUint{});
  EXPECT_EQ(x + BigUint{}, x);
  EXPECT_EQ(BigUint{} + x, x);
  EXPECT_EQ(x + x, (BigUint{0x0ULL, 0x3ULL}));
  EXPECT_EQ(x + y, (BigUint{0x0ULL, 0x0ULL, 0x1ULL}));
  EXPECT_EQ(y + x, (BigUint{0x0ULL, 0x0ULL, 0x1ULL}));
  EXPECT_EQ(x + z, (BigUint{0x8000000000000334ULL, 0x1ULL}));
  EXPECT_EQ(z + x, (BigUint{0x8000000000000334ULL, 0x1ULL}));
  EXPECT_EQ(z + z, BigUint{0x668ULL});
}

TEST(BigUint, Sub) {
  const BigUint x{0x0ULL, 0x2ULL};
  const BigUint y{0x1ULL, 0x1ULL};
  const BigUint z{0x0ULL, 0x1ULL, 0x1ULL};

  EXPECT_EQ(BigUint{} - BigUint{}, BigUint{});
  EXPECT_EQ(x - BigUint{}, x);
  EXPECT_EQ(x - x, BigUint({}));
  EXPECT_THROW(BigUint{} - x, std::out_of_range);
  EXPECT_EQ(x - y, BigUint{0xFFFFFFFFFFFFFFFFULL});
  EXPECT_THROW(y - x, std::out_of_range);
  EXPECT_EQ(z - y, (BigUint{0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL}));
}

TEST(BigUint, Mul) {
  const BigUint x{0x334ULL, 0x264ULL};
  const BigUint y{0x100000000ULL};

  EXPECT_EQ(BigUint{} * BigUint{}, BigUint{});
  EXPECT_EQ(x * BigUint{}, BigUint{});
  EXPECT_EQ(BigUint{} * x, BigUint{});
  EXPECT_EQ(x * x, (BigUint{0xA4290ULL, 0xF50A0ULL, 0x5B710ULL}));
  EXPECT_EQ(x * y, (BigUint{0x33400000000ULL, 0x26400000000ULL}));
  EXPECT_EQ(y * y, (BigUint{0x0ULL, 0x1ULL}));
}

TEST(BigUint, Increment) {
  BigUint x{};
  ++x;
  EXPECT_EQ(x, BigUint{0x1ULL});

  BigUint y{0x334ULL};
  ++y;
  EXPECT_EQ(y, BigUint{0x335ULL});

  BigUint z{0xFFFFFFFFFFFFFFFFULL};
  ++z;
  EXPECT_EQ(z, (BigUint{0x0ULL, 0x1ULL}));
}

TEST(BigUint, Shr) {
  BigUint x{0x4334334334334334ULL, 0x33};

  EXPECT_EQ(x >> 0, x);
  EXPECT_EQ(x >> 8, BigUint{0x3343343343343343ULL});
  EXPECT_EQ(x >> 64, BigUint{0x33ULL});
  EXPECT_EQ(x >> 128, BigUint{});
}

TEST(BigUint, Shl) {
  BigUint x{0x4334334334334334ULL, 0x33ULL};

  EXPECT_EQ(x << 0, x);
  EXPECT_EQ(x << 8, (BigUint{0x3433433433433400ULL, 0x3343ULL}));
  EXPECT_EQ(x << 64, (BigUint{0x0ULL, 0x4334334334334334ULL, 0x33ULL}));
}

TEST(BigUint, Comparison) {
  const BigUint x{0x33ULL, 0x4ULL};
  const BigUint y{0x264ULL};

  EXPECT_TRUE(x == x);
  EXPECT_TRUE(x != y);
  EXPECT_TRUE(x > y);
}

TEST(BigUint, ModAssign2Pow) {
  BigUint x{0x4334334334334334ULL, 0x33ULL};

  auto y0 = x;
  y0.ModAssign2Pow(0);
  EXPECT_EQ(y0, BigUint{});

  auto y1 = x;
  y1.ModAssign2Pow(64);
  EXPECT_EQ(y1, BigUint{0x4334334334334334ULL});

  auto y2 = x;
  y2.ModAssign2Pow(65);
  EXPECT_EQ(y2, (BigUint{0x4334334334334334ULL, 0x1ULL}));

  auto y3 = x;
  y3.ModAssign2Pow(256);
  EXPECT_EQ(y3, x);
}

TEST(BigUint, AddAssign2Pow) {
  const BigUint x{0x8000000000000000ULL};

  auto y0 = x;
  y0.AddAssign2Pow(0);
  EXPECT_EQ(y0, BigUint{0x8000000000000001ULL});

  auto y1 = x;
  y1.AddAssign2Pow(63);
  EXPECT_EQ(y1, (BigUint{0x0ULL, 0x1ULL}));

  auto y2 = x;
  y2.AddAssign2Pow(64);
  EXPECT_EQ(y2, (BigUint{0x8000000000000000ULL, 0x1ULL}));
}

TEST(BigUint, ShlAddAssign) {
  const BigUint x{0x334ULL};
  const BigUint y{0x264ULL};

  auto z1 = x;
  z1.ShlAddAssign(y, 0);
  EXPECT_EQ(z1, BigUint{0x598ULL});

  auto z2 = x;
  z2.ShlAddAssign(y, 1);
  EXPECT_EQ(z2, BigUint{0x334 + 0x264 * 2});

  auto z3 = x;
  z3.ShlAddAssign(y, 64);
  EXPECT_EQ(z3, (BigUint{0x334ULL, 0x264ULL}));

  auto z4 = x;
  z4.ShlAddAssign(y, 65);
  EXPECT_EQ(z4, (BigUint{0x334ULL, 0x264ULL * 2}));

  auto z5 = x;
  z5.ShlAddAssign(y, 128);
  EXPECT_EQ(z5, (BigUint{0x334ULL, 0x0ULL, 0x264ULL}));
}

TEST(BigUint, ShiftMod2Pow) {
  const BigUint x{0x1234567890abcdefULL, 0xfedcba0987654321ULL};

  for (std::size_t i = 0; i < 256 + 1; ++i) {
    for (std::size_t j = 0; j < 256 + 1; ++j) {
      auto y = x;
      y >>= i;
      y.ModAssign2Pow(j);
      EXPECT_EQ(x.ShiftMod2Pow(i, j), y) << i << " " << j;
    }
  }
}
