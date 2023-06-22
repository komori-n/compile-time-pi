#ifndef KOMORI_BIGINT_HPP_
#define KOMORI_BIGINT_HPP_

#include "biguint.hpp"

namespace komori {
class BigInt {
 public:
  constexpr BigInt(uint64_t value, Sign sign) : value_{value}, sign_{sign} {}
  constexpr BigInt(std::vector<uint64_t> values, Sign sign) : value_{std::move(values)}, sign_{sign} {}
  constexpr BigInt(std::initializer_list<uint64_t> values, Sign sign) : value_{std::move(values)}, sign_{sign} {}
  constexpr BigInt(BigUint value, Sign sign) : value_{std::move(value)}, sign_{sign} {}
  explicit constexpr BigInt(uint64_t value) : BigInt(value, Sign::kPositive) {}
  explicit constexpr BigInt(std::vector<uint64_t> value) : BigInt(std::move(value), Sign::kPositive) {}
  explicit constexpr BigInt(std::initializer_list<uint64_t> value) : BigInt(std::move(value), Sign::kPositive) {}
  explicit constexpr BigInt(BigUint value) : BigInt(std::move(value), Sign::kPositive) {}

  constexpr BigInt() = default;
  constexpr BigInt(const BigInt&) = default;
  constexpr BigInt(BigInt&&) noexcept = default;
  constexpr BigInt& operator=(const BigInt&) = default;
  constexpr BigInt& operator=(BigInt&&) noexcept = default;
  constexpr ~BigInt() = default;

  constexpr bool IsZero() const noexcept { return value_.IsZero(); }

  std::string DebugString() const {
    const auto sign_string = (sign_ == Sign::kPositive) ? "+" : "-";
    return sign_string + value_.DebugString();
  }

  constexpr BigInt& operator+=(const BigInt& rhs) {
    if (sign_ == rhs.sign_) {
      value_ += rhs.value_;
    } else if (value_ >= rhs.value_) {
      value_ -= rhs.value_;
    } else {
      value_ = rhs.value_ - std::move(value_);
      sign_ = rhs.sign_;
    }

    return *this;
  }

  constexpr BigInt& operator-=(const BigInt& rhs) {
    if (sign_ != rhs.sign_) {
      value_ += rhs.value_;
    } else if (value_ >= rhs.value_) {
      value_ -= rhs.value_;
    } else {
      value_ = rhs.value_ - std::move(value_);
      sign_ = ~sign_;
    }

    return *this;
  }

  constexpr BigInt& operator*=(const BigInt& rhs) {
    value_ *= rhs.value_;
    sign_ = sign_ ^ rhs.sign_;
    return *this;
  }

  constexpr BigInt& operator>>=(const std::size_t rhs) {
    value_ >>= rhs;
    return *this;
  }

  constexpr BigInt& operator<<=(const std::size_t rhs) {
    value_ <<= rhs;
    return *this;
  }

  friend constexpr BigInt operator+(const BigInt& lhs, const BigInt& rhs) {
    BigInt tmp = lhs;
    tmp += rhs;
    return tmp;
  }

  friend constexpr BigInt operator-(const BigInt& lhs, const BigInt& rhs) {
    BigInt tmp = lhs;
    tmp -= rhs;
    return tmp;
  }

  friend constexpr BigInt operator-(const BigInt& rhs) { return {rhs.value_, ~rhs.sign_}; }

  friend constexpr BigInt operator*(const BigInt& lhs, const BigInt& rhs) {
    BigInt tmp = lhs;
    tmp *= rhs;
    return tmp;
  }

  friend constexpr BigInt operator>>(const BigInt& lhs, const std::size_t& rhs) {
    return {lhs.value_ >> rhs, lhs.sign_};
  }
  friend constexpr BigInt operator<<(const BigInt& lhs, const std::size_t& rhs) {
    return {lhs.value_ << rhs, lhs.sign_};
  }
  friend constexpr std::strong_ordering operator<=>(const BigInt& lhs, const BigInt& rhs) {
    if (lhs.sign_ == Sign::kPositive && rhs.sign_ == Sign::kPositive) {
      return lhs.value_ <=> rhs.value_;
    } else if (lhs.sign_ == Sign::kNegative && rhs.sign_ == Sign::kNegative) {
      const auto ordering = lhs.value_ <=> rhs.value_;
      if (ordering == std::strong_ordering::greater) {
        return std::strong_ordering::less;
      } else if (ordering == std::strong_ordering::less) {
        return std::strong_ordering::greater;
      } else {
        return std::strong_ordering::equal;
      }
    } else if (lhs.sign_ == Sign::kPositive && rhs.sign_ == Sign::kNegative) {
      return std::strong_ordering::greater;
    } else {
      // lhs.sign_ == Sign::kNegative && rhs.sign_ == Sign::kPoisitive
      return std::strong_ordering::less;
    }
  }

  friend constexpr bool operator==(const BigInt& lhs, const BigInt& rhs) = default;

 private:
  BigUint value_{};
  Sign sign_{Sign::kPositive};
};
}  // namespace komori

#endif  // KOMORI_BIGINT_HPP_