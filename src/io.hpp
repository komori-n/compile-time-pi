#ifndef KOMORI_IO_HPP_
#define KOMORI_IO_HPP_

#include <string>

#include "komori/bigfloat.hpp"
#include "komori/biguint.hpp"

namespace komori {
namespace detail {
inline constexpr BigUint Make10Pow(uint64_t n) {
  return BigUint{10}.Pow(n);
}

inline constexpr int64_t Log10Int(const BigUint& num) {
  if (num.IsZero()) {
    throw std::out_of_range("The number must be greater than 0");
  }

  int64_t r = 1;
  while (Make10Pow(r) <= num) {
    r *= 2;
  }

  int64_t l = r / 2;
  while (r - l > 1) {
    const auto m = (l + r) / 2;
    if (Make10Pow(m) <= num) {
      l = m;
    } else {
      r = m;
    }
  }

  return l;
}

inline std::string FractionalPartToString(BigFloat num, int64_t digit_len) {
  const auto orig_precision = num.GetPrecision();

  if (digit_len <= 0) {
    return std::string{};
  } else if (digit_len <= 19) {
    num *= BigFloat(orig_precision, Make10Pow(digit_len));
    const auto value = static_cast<uint64_t>(num.IntegerPart());

    std::string ans = std::to_string(value);
    std::string zeros(digit_len - ans.length(), '0');
    return std::move(zeros) + std::move(ans);
  }

  const auto upper_part_len = digit_len / 2;
  const auto lower_part_len = digit_len - upper_part_len;

  auto upper_part_str = FractionalPartToString(num, upper_part_len);
  num *= BigFloat(orig_precision, Make10Pow(upper_part_len));
  auto lower_part_str = FractionalPartToString(num.FractionalPart(), lower_part_len);

  return std::move(upper_part_str) + std::move(lower_part_str);
}
}  // namespace detail

inline constexpr std::string ToString(const BigUint& num) {
  if (num.IsZero()) {
    return std::string{"0"};
  }

  const auto digit_len = detail::Log10Int(num) + 1;
  const auto number_of_bits = num.NumberOfBits();

  BigFloat b = BigFloat(number_of_bits + 10, detail::Make10Pow(digit_len));
  BigFloat inv_b = Inverse(b);
  BigFloat f = BigFloat(number_of_bits + 10, num) * inv_b;
  // Add `inv_b / 2` to round the result
  f = std::move(f) + (inv_b >> 2);

  return detail::FractionalPartToString(std::move(f), digit_len);
}

inline constexpr std::string ToString(const BigFloat& num) {
  constexpr double log2_10 = 3.321928094887362;

  auto integer_part = num.IntegerPart();
  auto fractional_part = num.FractionalPart();

  auto integer_part_str = ToString(integer_part);
  const auto frac_precision = fractional_part.GetFractionalPartPrecision();
  const auto digit_len = static_cast<int64_t>(std::floor(static_cast<double>(frac_precision) / log2_10));
  auto fractional_part_str = detail::FractionalPartToString(fractional_part, digit_len);

  return std::move(integer_part_str) + "." + std::move(fractional_part_str);
}
}  // namespace komori

#endif  // KOMORI_IO_HPP_