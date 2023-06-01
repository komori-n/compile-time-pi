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
  const BigFloat x(334);
  const auto res_x = x.FractionalPart();
  EXPECT_EQ(res_x.first, BigUint{});
  EXPECT_EQ(res_x.second, 0);

  const BigFloat y(334, BigUint({0x334}), 20);
  const auto res_y = y.FractionalPart();
  EXPECT_EQ(res_y.first, BigUint{});
  EXPECT_EQ(res_y.second, 0);

  const BigFloat z(334, BigUint({0x334}), -8);
  const auto res_z = z.FractionalPart();
  EXPECT_EQ(res_z.first, BigUint{0x34});
  EXPECT_EQ(res_z.second, -8);

  const BigFloat w(334, BigUint({0x334}), -100);
  const auto res_w = w.FractionalPart();
  EXPECT_EQ(res_w.first, BigUint{0x334});
  EXPECT_EQ(res_w.second, -100);
}