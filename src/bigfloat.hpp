#ifndef KOMORI_BIGFLOAT_HPP_
#define KOMORI_BIGFLOAT_HPP_

#include "komori/biguint.hpp"

namespace komori {
class BigFloat {
 public:
  constexpr explicit BigFloat(int64_t precision, BigUint significand = BigUint{}, int64_t exponent = 0)
      : precision_{precision}, significand_{std::move(significand)}, exponent_{exponent} {}
  constexpr BigFloat() = delete;
  constexpr BigFloat(const BigFloat&) = delete;
  constexpr BigFloat(BigFloat&&) noexcept = delete;
  constexpr BigFloat& operator=(const BigFloat&) = delete;
  constexpr BigFloat& operator=(BigFloat&&) noexcept = delete;
  constexpr ~BigFloat() = default;

  friend constexpr BigFloat operator+(BigFloat lhs, BigFloat rhs) {
    if (lhs.exponent_ < rhs.exponent_) {
      rhs.ExtendSignificand(lhs.exponent_);
    } else if (lhs.exponent_ > rhs.exponent_) {
      lhs.ExtendSignificand(rhs.exponent_);
    }

    const auto lowest_reliable_bit = std::max(lhs.LowestReliableBit(), rhs.LowestReliableBit());

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

    lhs.precision_ = static_cast<int64_t>(lhs.significand_.NumberOfBits()) - lowest_reliable_bit;

    return lhs;
  }

 private:
  constexpr void ExtendSignificand(int64_t exponent) {
    significand_ <<= exponent_ - exponent;
    exponent_ = exponent;
  }

  constexpr int64_t LowestReliableBit() const {
    const auto bit_width = static_cast<int64_t>(significand_.NumberOfBits());
    return bit_width - precision_;
  }

  enum class Sign : uint8_t {
    kPositive,
    kNegative,
  };

  int64_t precision_{};

  BigUint significand_{};
  int64_t exponent_{0};
  Sign sign_{Sign::kPositive};
};
}  // namespace komori

#endif  // KOMORI_BIGFLOAT_HPP_