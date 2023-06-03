#include <gtest/gtest.h>

#include <array>
#include <random>
#include "komori/ssa.hpp"

using komori::BigUint;
using komori::detail::SplittedInteger;

TEST(SplittedInteger, Basic) {
  const BigUint x{0x1234567890abcdefULL};
  const BigUint y{0x1234567890abcdefULL, 0xfedcba0987654321ULL};
  EXPECT_EQ(x, SplittedInteger(x, 5).Get());
  EXPECT_EQ(y, SplittedInteger(y, 6).Get());
}

TEST(SplittedInteger, NTT) {
  const BigUint x{0x1234567890abcdefULL};

  SplittedInteger splitted_x(x, 5);
  splitted_x.NTT();
  splitted_x.INTT();
  EXPECT_EQ(x, splitted_x.Get());

  const BigUint y{0x1234567890abcdefULL, 0xfedcba0987654321ULL};
  SplittedInteger splitted_y(y, 6);
  splitted_y.NTT();
  splitted_y.INTT();
  EXPECT_EQ(y, splitted_y.Get());
}

TEST(SplittedInteger, Multiply) {
  using komori::detail::MultiplySSA;

  std::vector<uint64_t> x_vec;
  std::vector<uint64_t> y_vec;

  std::mt19937_64 mt(334);
  std::uniform_int_distribution<std::uint64_t> dist;
  for (std::size_t i = 0; i < 100; ++i) {
    x_vec.push_back(dist(mt));
    y_vec.push_back(dist(mt));
  }

  const BigUint x{std::move(x_vec)};
  const BigUint y{std::move(y_vec)};
  const auto naive_ans = MultiplyNaive(x, y);
  const auto karatsuba_ans = MultiplyKaratsuba(x, y);
  const auto ssa_ans = MultiplySSA(x, y);

  EXPECT_EQ(naive_ans, karatsuba_ans);
  EXPECT_EQ(naive_ans, ssa_ans);
}