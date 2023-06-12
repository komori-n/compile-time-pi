#ifndef KOMORI_BIGFLOAT_HPP_
#define KOMORI_BIGFLOAT_HPP_

#include "komori/biguint.hpp"
#include "komori/ssa.hpp"

namespace komori {
/**
 * @brief An arbitrary precision floating number
 * @detail
 * This class represents a real number by the following form:
 *    (sign) * significand * 2^(exponent)
 */
class BigFloat {
 public:
  /// Sign of the number. Note that all numbers including zero is considered to be positive or negative. Zero has two
  /// representation: +0 and -0.
  enum class Sign : uint8_t {
    kPositive,  ///< Positive
    kNegative,  ///< Negative
  };

  /// Flip a sign
  friend constexpr Sign operator~(Sign lhs) noexcept {
    return lhs == Sign::kPositive ? Sign::kNegative : Sign::kPositive;
  }

  /// Multiply signs
  friend constexpr Sign operator^(Sign lhs, Sign rhs) noexcept {
    return lhs == rhs ? Sign::kPositive : Sign::kNegative;
  }

  /**
   * @brief Constructor
   * @param precision The precision of the number
   * @param significand The significand part of the number. (default is zero)
   * @param exponent The exponent part of the number. (default is zero)
   */
  constexpr explicit BigFloat(int64_t precision, BigUint significand = BigUint{})
      : BigFloat(precision, std::move(significand), Sign::kPositive) {}
  constexpr BigFloat() = delete;
  constexpr BigFloat(const BigFloat&) = default;
  constexpr BigFloat(BigFloat&&) noexcept = default;
  constexpr BigFloat& operator=(const BigFloat&) = default;
  constexpr BigFloat& operator=(BigFloat&&) noexcept = default;
  constexpr ~BigFloat() = default;

  constexpr Sign GetSign() const noexcept { return sign_; }
  constexpr int64_t GetPrecision() const noexcept { return precision_; }
  constexpr void SetPrecision(int64_t precision) noexcept { precision_ = precision; }
  constexpr bool IsZero() const noexcept { return significand_.IsZero(); }

  constexpr int64_t GetFractionalPartPrecision() const noexcept {
    const auto reliable_bit_len = -(LowestReliableBit() + exponent_);
    return std::max<int64_t>(0, reliable_bit_len);
  }

  std::string DebugString() const {
    std::string s;
    if (sign_ == Sign::kNegative) {
      s += "-";
    }

    s += significand_.DebugString();
    s += " * 2^";
    if (exponent_ >= 0) {
      s += std::to_string(exponent_);
    } else {
      s += "(" + std::to_string(exponent_) + ")";
    }

    return s;
  }

  /**
   * @brief Add two numbers
   * @param lhs The first number to be added
   * @param rhs The second number to be added
   * @return The result of the addition
   * @detail We take the values instead of the (lvalue) references in order to keep the implementation simple.
   */
  friend constexpr BigFloat operator+(BigFloat lhs, BigFloat rhs) {
    // Make `exponent_` the same value. The one that has greater `exponent_` should be shifted.
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
      if (lhs.significand_ >= rhs.significand_) {
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
    exponent_ += rhs.exponent_;

    Simplify();
    return *this;
  }

  friend constexpr BigFloat operator*(BigFloat lhs, BigFloat rhs) {
    lhs *= std::move(rhs);
    return lhs;
  }

  /**
   * @brief Left-shift by `rhs` bits and assign to self
   * @param rhs The number of bits to be shifted. This can be negative.
   * @return *this
   * @detail If `rhs` is negative, the behaviors of this method is like `operator>>=(-rhs)`
   */
  constexpr BigFloat& operator<<=(int64_t rhs) noexcept {
    exponent_ += rhs;
    return *this;
  }

  /**
   * @brief Left-shift by `rhs` bits
   * @param rhs The number of bits to be shifted. This can be negative.
   * @return Result of the calculation
   * @detail If `rhs` is negative, the behaviors of this method is like `operator>>(-rhs)`
   */
  friend constexpr BigFloat operator<<(BigFloat lhs, int64_t rhs) {
    lhs <<= rhs;
    return lhs;
  }

  /**
   * @brief Right-shift by `rhs` bits and assign to self
   * @param rhs The number of bits to be shifted. This can be negative.
   * @return *this
   * @detail If `rhs` is negative, the behaviors of this method is like `operator<<=(-rhs)`
   */
  constexpr BigFloat& operator>>=(int64_t rhs) noexcept {
    exponent_ -= rhs;
    return *this;
  }

  /**
   * @brief Right-shift by `rhs` bits
   * @param rhs The number of bits to be shifted. This can be negative.
   * @return Result of the calculation
   * @detail If `rhs` is negative, the behaviors of this method is like `operator<<(-rhs)`
   */
  friend constexpr BigFloat operator>>(BigFloat lhs, int64_t rhs) {
    lhs >>= rhs;
    return lhs;
  }

  /**
   * @brief Get the integer part of the abs of the number
   * @return The integer part
   */
  constexpr BigUint IntegerPart() const {
    if (exponent_ > 0) {
      return significand_ << exponent_;
    } else {
      return significand_ >> (-exponent_);
    }
  }

  /**
   * @brief Get the fractional part of the abs of the number
   * @return The fractional part
   */
  constexpr BigFloat FractionalPart() const {
    const auto dot_bit = -exponent_;
    if (dot_bit < LowestReliableBit()) {
      return BigFloat{0};
    }

    auto ans_precision = dot_bit - LowestReliableBit();
    auto ans_sign = sign_;
    if (dot_bit <= 0) {
      // The fractional part is obviously zero
      return BigFloat{ans_precision, BigUint{}, ans_sign};
    }
    auto ans_significand = significand_.ShiftMod2Pow(0, dot_bit);
    return BigFloat{ans_precision, std::move(ans_significand), ans_sign} >> dot_bit;
  }

  constexpr BigFloat ApproximateInverse() const {
    BigUint tmp = significand_;
    const auto bit_width = tmp.NumberOfBits();
    int64_t exp = exponent_;
    if (bit_width > 32) {
      tmp >>= bit_width - 32;
      exp += bit_width - 32;
    }

    const uint64_t value = static_cast<uint64_t>(tmp);
    if (value == 0) {
      throw std::range_error("The divisor is zero");
    }

    const auto precision = std::min<std::int64_t>(32, precision_);
    if (value == 1) {
      return BigFloat(precision, BigUint{1}, sign_) >> exp;
    } else {
      const auto approx_div = static_cast<uint64_t>((uint128_t{1} << 64) / value);
      return BigFloat(precision, BigUint{approx_div}, sign_) >> (64 + exp);
    }
  }

  constexpr BigFloat ApproximateSqrt() const {
    BigUint tmp = significand_;
    const auto bit_width = tmp.NumberOfBits();
    int64_t exp = exponent_;

    if (bit_width > 64) {
      tmp >>= bit_width - 64;
      exp += bit_width - 64;
    }

    if (exp % 2) {
      tmp >>= 1;
      ++exp;
    }

    const uint64_t value = ISqrt(static_cast<uint64_t>(tmp));
    const auto precision = std::min<std::int64_t>(31, precision_ / 2);

    return BigFloat(precision, BigUint{value}) << (exp / 2);
  }

 private:
  constexpr explicit BigFloat(int64_t precision, BigUint significand, Sign sign)
      : precision_{precision}, significand_{std::move(significand)}, sign_{sign} {}

  /**
   * @brief Multiply 2 ** `exponent_ - exponent` to the significand part, and set `exponent` to the exponent part
   * @param exponent The exponent part after the conversion
   * @pre exponent < exponent_
   */
  constexpr void ExtendSignificand(int64_t exponent) {
    significand_ <<= exponent_ - exponent;
    exponent_ = exponent;
  }

  constexpr int64_t LowestReliableBit() const {
    const auto bit_width = static_cast<int64_t>(significand_.NumberOfBits());
    return bit_width - precision_;
  }

  constexpr void Simplify() {
    if (precision_ <= 0) {
      // The number of reliable bits is zero, but the value is less than 2^exponent_
      exponent_ -= precision_;
      precision_ = 0;
      significand_ = BigUint{};
    } else {
      const auto lowest_reliable_bit = LowestReliableBit();
      if (lowest_reliable_bit > 64) {
        const auto shift = lowest_reliable_bit - 1;
        significand_ >>= shift;
        exponent_ += shift;
      }
    }
  }

  /// The number of reliable digits (precision) of the number. The value may be greater or less than the bid width of
  /// `significand_`.
  ///
  /// We use a signed type instead of an unsigned type because it's frequently used with `exponent_`, which is signed.
  int64_t precision_{};

  /// The significand part of the number
  BigUint significand_{};
  /// The exponent part of the number
  int64_t exponent_{0};
  /// The sign of the number
  Sign sign_{Sign::kPositive};
};

constexpr inline BigFloat Inverse(BigFloat num) {
  const auto target_precision = num.GetPrecision();
  BigFloat a = num.ApproximateInverse();

  while (a.GetPrecision() < target_precision) {
    a.SetPrecision(2 * a.GetPrecision());
    auto x = BigFloat(target_precision, BigUint{1}) - num * a;
    x *= a;
    a.SetPrecision(a.GetPrecision() - 1);
    a = std::move(a) + std::move(x);
  }

  return a;
}

constexpr inline BigFloat operator/(BigFloat lhs, BigFloat rhs) {
  return std::move(lhs) * Inverse(std::move(rhs));
}

constexpr inline BigFloat SqrtInverse(BigFloat num) {
  const auto target_precision = num.GetPrecision();
  if (num.GetSign() == BigFloat::Sign::kNegative) {
    throw std::out_of_range("The number must not be negative");
  }

  BigFloat a = Inverse(num.ApproximateSqrt());

  while (a.GetPrecision() < target_precision) {
    a.SetPrecision(2 * a.GetPrecision());
    auto x = BigFloat(target_precision, BigUint{1}) - num * a * a;
    x = (a * x) >> 1;
    a.SetPrecision(a.GetPrecision() - 1);
    a = std::move(a) + std::move(x);
  }

  return a;
}

constexpr inline BigFloat Sqrt(BigFloat num) {
  if (num.IsZero()) {
    return BigFloat(num.GetPrecision(), BigUint{0});
  }

  auto sqrt_inv = SqrtInverse(num);
  return std::move(num) * std::move(sqrt_inv);
}
}  // namespace komori

#endif  // KOMORI_BIGFLOAT_HPP_