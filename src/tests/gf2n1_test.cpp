#include <gtest/gtest.h>

#include "komori/gf2n1.hpp"

using komori::BigUint;
using komori::GF2PowNPlus1;
using komori::uint128_t;

constexpr BigUint MakeBigUint(uint64_t value) noexcept {
  std::vector<uint64_t> vec;
  if (value > 0) {
    vec.push_back(value);
  }

  return BigUint{std::move(vec)};
}

TEST(GF2PowNPlus1, ConstructByDynamicBigUint) {
  EXPECT_EQ(GF2PowNPlus1(64, BigUint{0x334ULL}).Get(), BigUint{0x334ULL});
  EXPECT_EQ(GF2PowNPlus1(8, BigUint{0x33ULL, 0x4ULL}).Get(), (BigUint{0x33ULL, 0x4ULL}));
}

TEST(GF2PowNPlus1, Make2Pow) {
  for (std::size_t p = 0; p < 64; ++p) {
    const auto x = GF2PowNPlus1::Make2Pow(p, 8);
    const std::uint64_t expected = static_cast<uint64_t>((uint128_t{1} << p) % 257);
    EXPECT_EQ(x.Get(), BigUint{expected}) << p;
  }
}

TEST(GF2PowNPlus1, Add) {
  for (std::uint64_t i = 0; i < 256 + 1; ++i) {
    GF2PowNPlus1 x(8, MakeBigUint(i));
    for (std::uint64_t j = 0; j < 256 + 1; ++j) {
      GF2PowNPlus1 y(8, MakeBigUint(j));

      EXPECT_EQ((x + y).Get(), MakeBigUint((i + j) % 257));
    }
  }
}

TEST(GF2PowNPlus1, Sub) {
  for (std::uint64_t i = 0; i < 256 + 1; ++i) {
    GF2PowNPlus1 x(8, MakeBigUint(i));
    for (std::uint64_t j = 0; j < 256 + 1; ++j) {
      GF2PowNPlus1 y(8, MakeBigUint(j));

      EXPECT_EQ((x - y).Get(), MakeBigUint((257 + i - j) % 257));
    }
  }
}

TEST(GF2PowNPlus1, Mul) {
  for (std::uint64_t i = 0; i < 256 + 1; ++i) {
    GF2PowNPlus1 x(8, MakeBigUint(i));
    for (std::uint64_t j = 0; j < 256 + 1; ++j) {
      GF2PowNPlus1 y(8, MakeBigUint(j));

      EXPECT_EQ((x * y).Get(), MakeBigUint((i * j) % 257));
    }
  }
}