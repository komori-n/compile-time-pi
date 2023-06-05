#ifndef KOMORI_COMMON_HPP_
#define KOMORI_COMMON_HPP_

#include <concepts>
#include <limits>

namespace komori {
using uint64_t = std::uint64_t;
using uint128_t = __uint128_t;

template <typename T>
struct DefineNotEqualByEqual {
  friend constexpr bool operator!=(const T& lhs, const T& rhs) noexcept(noexcept(lhs == rhs)) { return !(lhs == rhs); }
};

template <typename T>
struct DefineComparisonOperatorsByLess {
  constexpr friend bool operator<=(const T& lhs, const T& rhs) noexcept(noexcept(lhs < rhs)) { return !(rhs < lhs); }
  constexpr friend bool operator>(const T& lhs, const T& rhs) noexcept(noexcept(rhs < lhs)) { return rhs < lhs; }
  constexpr friend bool operator>=(const T& lhs, const T& rhs) noexcept(noexcept(lhs < rhs)) { return !(lhs < rhs); }
};

template <std::integral T>
constexpr T DivCeil(T value, std::type_identity_t<T> div) noexcept {
  return (value + div - 1) / div;
}

constexpr inline uint64_t ISqrt(uint64_t value) {
  uint64_t r = value / 2;
  uint64_t l = std::numeric_limits<uint64_t>::min();
  while (r - l > 1) {
    const uint128_t m = static_cast<uint128_t>((l + r) / 2);
    if (m * m <= static_cast<uint128_t>(value)) {
      l = m;
    } else {
      r = m;
    }
  }

  return l;
}

}  // namespace komori

#endif  // KOMORI_COMMON_HPP_