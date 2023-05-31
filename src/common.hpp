#ifndef KOMORI_COMMON_HPP_
#define KOMORI_COMMON_HPP_

#include <concepts>

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

}  // namespace komori

#endif  // KOMORI_COMMON_HPP_