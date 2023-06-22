#ifndef KOMORI_COMMON_HPP_
#define KOMORI_COMMON_HPP_

#include <concepts>
#include <limits>

namespace komori {
using uint64_t = std::uint64_t;
using uint128_t = __uint128_t;

/// Sign of the number. Note that all numbers including zero is considered to be positive or negative. Zero has two
/// representation: +0 and -0.
enum class Sign : uint8_t {
  kPositive,  ///< Positive
  kNegative,  ///< Negative
};

/// Flip a sign
constexpr Sign operator~(Sign lhs) noexcept {
  return lhs == Sign::kPositive ? Sign::kNegative : Sign::kPositive;
}

/// Multiply signs
constexpr Sign operator^(Sign lhs, Sign rhs) noexcept {
  return lhs == rhs ? Sign::kPositive : Sign::kNegative;
}

/**
 * @brief Divide `value` by `div` and ceil the result
 * @tparam T An integer type
 * @param value The dividend
 * @param div The divisor
 * @return the result
 */
template <std::integral T>
constexpr T DivCeil(T value, std::type_identity_t<T> div) noexcept {
  return (value + div - 1) / div;
}

/**
 * @brief Calculate the integer square root of `value`
 * @param value An integer
 * @return The integer square root of `value`
 */
constexpr inline uint64_t ISqrt(uint64_t value) {
  const uint128_t value_u128 = static_cast<uint128_t>(value);
  uint64_t r = value / 2;
  uint64_t l = std::numeric_limits<uint64_t>::min();
  while (r - l > 1) {
    const uint128_t m = static_cast<uint128_t>((l + r) / 2);
    if (m * m <= value_u128) {
      l = m;
    } else {
      r = m;
    }
  }

  return l;
}
}  // namespace komori

#endif  // KOMORI_COMMON_HPP_