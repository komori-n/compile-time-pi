#ifndef KOMORI_BIGFLOAT_HPP_
#define KOMORI_BIGFLOAT_HPP_

#include "komori/biguint.hpp"

namespace komori {
class BigFloat {
 public:
  constexpr explicit BigFloat(uint64_t precision_words, BigUint significand = BigUint{}, int64_t exponent = 0)
      : precision_{precision_words}, significand_{std::move(significand)}, exponent_{exponent} {}
  constexpr BigFloat() = delete;
  constexpr BigFloat(const BigFloat&) = delete;
  constexpr BigFloat(BigFloat&&) noexcept = delete;
  constexpr BigFloat& operator=(const BigFloat&) = delete;
  constexpr BigFloat& operator=(BigFloat&&) noexcept = delete;
  constexpr ~BigFloat() = default;

  friend constexpr BigFloat operator+(BigFloat lhs, BigFloat rhs) {
    if (lhs.exponent_ < rhs.exponent_) {
      lhs.ModifyExponent(rhs.exponent_);
    } else if (lhs.exponent_ > rhs.exponent_) {
      rhs.ModifyExponent(lhs.exponent_);
    }

    // Calculate lhs += rhs. `rhs` may be moved during the addition.
    if (lhs.sign_ == rhs.sign_) {
      lhs.significand_ += rhs.significand_;
    } else {
      if (lhs.significand_ > rhs.significand_) {
        lhs.significand_ -= rhs.significand_;
      } else {
        rhs.significand_ -= lhs.significand_;
        lhs = std::move(rhs);
      }
    }

    // TODO: Adjust result based on precision
    // TODO: Adjust precision

    return lhs;
  }

 private:
  constexpr void ModifyExponent(int64_t precision) {}

  enum class Sign : uint8_t {
    kPositive,
    kNegative,
  };

  uint64_t precision_{};

  BigUint significand_{};
  int64_t exponent_{0};
  Sign sign_{Sign::kPositive};
};
}  // namespace komori

#endif  // KOMORI_BIGFLOAT_HPP_