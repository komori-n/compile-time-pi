#ifndef KOMORI_BIGFLOAT_HPP_
#define KOMORI_BIGFLOAT_HPP_

#include "komori/biguint.hpp"
#include "komori/ssa.hpp"

namespace komori {
class BigFloat {
 public:
  constexpr explicit BigFloat(int64_t precision, BigUint significand = BigUint{}, int64_t exponent = 0)
      : precision_{precision}, significand_{std::move(significand)}, exponent_{exponent} {}
  constexpr BigFloat() = delete;
  constexpr BigFloat(const BigFloat&) = delete;
  constexpr BigFloat(BigFloat&&) noexcept = default;
  constexpr BigFloat& operator=(const BigFloat&) = delete;
  constexpr BigFloat& operator=(BigFloat&&) noexcept = default;
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
    lhs.Simplify();

    return lhs;
  }

  friend constexpr BigFloat operator-(BigFloat lhs, BigFloat rhs) { return operator+(std::move(lhs), -std::move(rhs)); }
  friend constexpr BigFloat operator-(BigFloat rhs) noexcept {
    rhs.sign_ = ~rhs.sign_;
    return rhs;
  }

  constexpr BigFloat& operator*=(const BigFloat& rhs) {
    significand_ = Multiply(significand_, rhs.significand_);
    sign_ = sign_ ^ rhs.sign_;
    precision_ = std::min(precision_, rhs.precision_);

    Simplify();
    return *this;
  }

  friend constexpr BigFloat operator*(BigFloat lhs, BigFloat rhs) {
    lhs *= std::move(rhs);
    return lhs;
  }

  friend constexpr std::weak_ordering operator<=>(const BigFloat& lhs, const BigFloat& rhs) noexcept {
    if (lhs.sign_ == Sign::kNegative && rhs.sign_ == Sign::kPositive) {
      return std::weak_ordering::less;
    } else if (lhs.sign_ == Sign::kPositive && rhs.sign_ == Sign::kNegative) {
      return std::weak_ordering::greater;
    } else {
      auto ans = std::strong_ordering::equal;
      if (lhs.exponent_ < rhs.exponent_) {
        ans = (lhs.significand_ >> (rhs.exponent_ - lhs.exponent_)) <=> rhs.significand_;
      } else {
        ans = lhs.significand_ <=> (rhs.significand_ >> (rhs.exponent_ - lhs.exponent_));
      }

      if (lhs.sign_ == Sign::kPositive) {
        return ans;
      }

      if (ans == std::strong_ordering::equal) {
        return std::weak_ordering::equivalent;
      } else if (ans == std::strong_ordering::less) {
        return std::weak_ordering::greater;
      } else {
        return std::weak_ordering::less;
      }
    }
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

  constexpr void Simplify() {
    const auto lowest_reliable_bit = LowestReliableBit();
    if (lowest_reliable_bit > 64) {
      const auto shift = lowest_reliable_bit - 1;
      significand_ >>= shift;
      exponent_ += shift;
    }
  }

  enum class Sign : uint8_t {
    kPositive,
    kNegative,
  };

  friend constexpr Sign operator~(Sign lhs) noexcept {
    return lhs == Sign::kPositive ? Sign::kNegative : Sign::kPositive;
  }

  friend constexpr Sign operator^(Sign lhs, Sign rhs) noexcept {
    return lhs == rhs ? Sign::kPositive : Sign::kNegative;
  }

  int64_t precision_{};

  BigUint significand_{};
  int64_t exponent_{0};
  Sign sign_{Sign::kPositive};
};
}  // namespace komori

#endif  // KOMORI_BIGFLOAT_HPP_