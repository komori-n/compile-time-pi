#ifndef KOMORI_GF2N1_HPP_
#define KOMORI_GF2N1_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "komori/biguint.hpp"

namespace komori {
class GF2PowNPlus1 {
 private:
  constexpr void ApplyMod() {
    // We want to get y(0 <= y < 2**n + 1) such that
    //   x = y (mod 2**n + 1)
    // If x is representable by
    //   x = q * 2**n + r  (0 <= q < 2**n + 1, 0 <= r < 2**n + 1)
    // then
    //   x = q * (2**n + 1) + (r - q)
    //     = r - q (mod 2**n + 1)
    // follows. Therefore, we will calculate
    //     r - q
    BigUint q = value_ >> n_;
    if (q.IsZero()) {
      return;
    }

    value_.ModAssign2Pow(n_);

    // Calculate ((value_ - q)  mod 2**n + 1)
    if (value_ < q) {
      AddModValue();
    }
    value_ -= q;
  }

 public:
  GF2PowNPlus1() = delete;
  explicit constexpr GF2PowNPlus1(std::size_t n) : n_{n}, value_{} {}
  constexpr GF2PowNPlus1(std::size_t n, BigUint value) : n_{n}, value_{std::move(value)} {}
  constexpr GF2PowNPlus1(const GF2PowNPlus1&) = default;
  constexpr GF2PowNPlus1(GF2PowNPlus1&&) noexcept = default;
  constexpr GF2PowNPlus1& operator=(const GF2PowNPlus1&) = default;
  constexpr GF2PowNPlus1& operator=(GF2PowNPlus1&&) noexcept = default;
  constexpr ~GF2PowNPlus1() = default;

  static constexpr GF2PowNPlus1 Make2Pow(std::size_t p, std::size_t n) {
    p = p % (2 * n);
    BigUint ans{1};
    ans <<= p;
    GF2PowNPlus1 ret(n, std::move(ans));
    if (p > n) {
      ret.ApplyMod();
    }
    return ret;
  }

  constexpr const BigUint& Get() const noexcept { return value_; }

  constexpr GF2PowNPlus1& operator+=(const GF2PowNPlus1& rhs) {
    if (n_ != rhs.n_) {
      throw std::invalid_argument("n mismatch");
    }
    value_ += rhs.value_;
    ApplyMod();
    return *this;
  }

  constexpr GF2PowNPlus1& operator-=(const GF2PowNPlus1& rhs) {
    if (n_ != rhs.n_) {
      throw std::invalid_argument("n mismatch");
    }

    if (value_ < rhs.value_) {
      AddModValue();
    }
    value_ -= rhs.value_;
    ApplyMod();
    return *this;
  }

  constexpr GF2PowNPlus1& operator*=(const GF2PowNPlus1& rhs) {
    if (n_ != rhs.n_) {
      throw std::invalid_argument("n mismatch");
    }
    value_ *= rhs.value_;
    ApplyMod();
    return *this;
  }

  friend constexpr GF2PowNPlus1 operator+(const GF2PowNPlus1& lhs, const GF2PowNPlus1& rhs) {
    GF2PowNPlus1 tmp = lhs;
    tmp += rhs;
    return tmp;
  }

  friend constexpr GF2PowNPlus1 operator-(const GF2PowNPlus1& lhs, const GF2PowNPlus1& rhs) {
    GF2PowNPlus1 tmp = lhs;
    tmp -= rhs;
    return tmp;
  }

  friend constexpr GF2PowNPlus1 operator*(const GF2PowNPlus1& lhs, const GF2PowNPlus1& rhs) {
    GF2PowNPlus1 tmp = lhs;
    tmp *= rhs;
    return tmp;
  }

 private:
  constexpr void AddModValue() {
    value_.AddAssign2Pow(n_);
    ++value_;
  }

  std::size_t n_;
  BigUint value_;
};
}  // namespace komori

#endif  // KOMORI_GF2N1_HPP_