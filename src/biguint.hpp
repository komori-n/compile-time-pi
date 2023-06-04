#ifndef KOMORI_BIGUINT_HPP_
#define KOMORI_BIGUINT_HPP_

#include <algorithm>
#include <bit>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "komori/common.hpp"

namespace komori {
class BigUint : private std::vector<uint64_t> {
  using Base = std::vector<uint64_t>;

 public:
  // <Constructors>
  /// Construct from a uint64 value
  constexpr explicit BigUint(uint64_t value) : Base{value} {}
  /// Construct from a vector
  constexpr explicit BigUint(std::vector<uint64_t> values) : Base{std::move(values)} { TrimLeadingZeros(); }
  /// Construct from a initializer list
  constexpr explicit BigUint(std::initializer_list<uint64_t> value) : Base{std::move(value)} { TrimLeadingZeros(); }

  constexpr BigUint() = default;
  constexpr BigUint(const BigUint&) = default;
  constexpr BigUint(BigUint&&) noexcept = default;
  constexpr BigUint& operator=(const BigUint&) = default;
  constexpr BigUint& operator=(BigUint&&) noexcept = default;
  constexpr ~BigUint() = default;
  // </Constructors>

  // <Basic Methods>
  /// Judge if the number is zero(Don't use `empty()` directly outside of this class)
  constexpr bool IsZero() const noexcept { return empty(); }

  /**
   * @brief Count the number of bits to represent *this
   * @return The number of bits to represent *this
   *
   * Examples:
   * ```cpp
   * BigUint{}.NumberOfBits();  // 0
   * BigUint{0x334}.NumberOfBits();  // 10
   * BigUint{0x0, 0x1}.NumberOfBits();  // 65
   * ```
   */
  constexpr uint64_t NumberOfBits() const {
    // Before calling `this->back()`, we must check if the number is zero.
    if (IsZero()) {
      return 0;
    }

    const uint64_t num_of_bits_back = std::bit_width(this->back());
    const uint64_t num_of_bits_except_back = (this->size() - 1) * 64;
    return num_of_bits_back + num_of_bits_except_back;
  }

  constexpr BigUint Pow(uint64_t index) const {
    if (index >= uint64_t{1} << 63) {
      throw std::out_of_range("The index is too big");
    }

    BigUint ans = BigUint{1};
    BigUint curr_base = *this;
    uint64_t curr_index_mask = 1;

    while (curr_index_mask <= index) {
      if (index & curr_index_mask) {
        ans *= curr_base;
      }

      curr_base *= curr_base;
      curr_index_mask <<= 1;
    }

    return ans;
  }

  /**
   * @brief Convert the value to uint64_t
   * @return The integer
   * @throw `std::range_error` if the number is greater than 2^64-1
   */
  constexpr explicit operator uint64_t() const {
    if (this->empty()) {
      return 0;
    } else if (this->size() == 1) {
      return this->back();
    } else if (this->size() > 1) {
      throw std::range_error("The number is too big");
    }
  }
  // </Basic Methods>

  // <Operators>
  constexpr BigUint& operator+=(const BigUint& rhs) {
    this->resize(std::max(this->size(), rhs.size()));

    uint128_t carry = 0;
    const auto len = this->size();
    for (std::size_t i = 0; i < len && (i < rhs.size() || carry > 0); ++i) {
      // We use uint128_t to handle overflows in the additions
      const auto lhs_value = static_cast<uint128_t>((*this)[i]);
      const auto rhs_value = (i < rhs.size()) ? static_cast<uint128_t>(rhs[i]) : 0;
      const auto sum = static_cast<uint128_t>(lhs_value) + static_cast<uint128_t>(rhs_value) + carry;

      // Store lower 64 bits
      (*this)[i] = static_cast<uint64_t>(sum);
      // Carry upper 64 bits
      carry = sum >> 64;
    }

    if (carry > 0) {
      this->push_back(carry);
    }

    return *this;
  }

  /**
   * @brief Subtract `rhs` and store the result to `this`
   * @param rhs right-hand side value
   * @return `*this` after the subtraction
   * @pre *this >= rhs
   */
  constexpr BigUint& operator-=(const BigUint& rhs) {
    if (*this < rhs) {
      throw std::out_of_range("`*this - rhs` must not be negative");
    }

    bool borrow = false;
    for (std::size_t i = 0; i < rhs.size(); ++i) {
      // (*this)[i] = (*this)[i] - uint64_t{borrow} - rhs[i]
      // We can judge wether `a - b` caused over flow by evaluating `a < a - b`.
      const auto result1 = (*this)[i] - static_cast<uint64_t>(borrow);
      borrow = (*this)[i] < result1;
      const auto result2 = result1 - rhs[i];
      borrow |= result1 < result2;

      (*this)[i] = result2;
    }

    if (borrow) {
      // Decrement values while the value is zero
      for (std::size_t i = rhs.size(); i < this->size(); ++i) {
        if ((*this)[i]-- > 0) {
          break;
        }
      }
    }

    TrimLeadingZeros();
    return *this;
  }

  constexpr BigUint& operator*=(const BigUint& rhs) {
    // The multiplication requires memory allocation, so we cannot define MulAssign operator efficiently. Therefore, we
    // use the implementation of the multiplication operator.
    *this = *this * rhs;
    return *this;
  }

  constexpr BigUint& operator>>=(const std::size_t& rhs) {
    const auto word_idx = rhs / 64;
    const auto bit_idx = rhs % 64;

    std::size_t new_size = 0;
    if (word_idx < this->size()) {
      if (bit_idx == 0) {
        std::move(this->begin() + word_idx, this->end(), this->begin());
        new_size = this->size() - word_idx;
      } else {
        for (std::size_t i = 0; i < this->size() - word_idx; ++i) {
          //                  i+word_idx+1     i + word_idx
          // <|----------------|----------------|
          //               ^                ^
          //            bit_idx           bit_idx
          //                |<------------->|
          //                new (*this)[i]
          auto word = (*this)[i + word_idx] >> bit_idx;
          if (i + word_idx + 1 < this->size()) {
            word |= (*this)[i + word_idx + 1] << (64 - bit_idx);
          }

          (*this)[i] = word;
        }

        new_size = std::min(this->size() - word_idx, this->size());
      }
    }

    this->resize(new_size);
    TrimLeadingZeros();
    return *this;
  }

  constexpr BigUint& operator<<=(const std::size_t& rhs) {
    const auto word_idx = rhs / 64;
    const auto bit_idx = rhs % 64;

    const std::size_t orig_len = this->size();
    this->resize(orig_len + word_idx + (bit_idx > 0 ? 1 : 0));

    if (bit_idx == 0) {
      // Move all values from last to first (in order not to overwrite values)
      for (std::size_t i = 0; i < orig_len; ++i) {
        const std::size_t src = orig_len - i - 1;
        const std::size_t dst = src + word_idx;
        if (dst < this->size()) {
          (*this)[dst] = (*this)[src];
        }
      }
    } else {
      for (std::size_t i = 0; i < orig_len; ++i) {
        const std::size_t src = orig_len - i - 1;
        const std::size_t dst_upper = src + word_idx + 1;
        const std::size_t dst_lower = src + word_idx;
        if (dst_upper < this->size()) {
          (*this)[dst_upper] |= (*this)[src] >> (64 - bit_idx);
        }
        if (dst_lower < this->size()) {
          (*this)[dst_lower] = (*this)[src] << bit_idx;
        }
      }
    }

    std::fill(this->begin(), this->begin() + std::min(word_idx, this->size()), 0);
    TrimLeadingZeros();
    return *this;
  }

  constexpr BigUint& operator++() {
    for (std::size_t i = 0; i < this->size(); ++i) {
      if (++(*this)[i] > 0) {
        return *this;
      }
    }

    this->push_back(1);
    return *this;
  }

  constexpr BigUint operator++(int) {
    BigUint ret = *this;
    operator++();
    return ret;
  }

  friend constexpr BigUint operator+(const BigUint& lhs, const BigUint& rhs) {
    BigUint tmp = lhs;
    tmp += rhs;
    return tmp;
  }

  friend constexpr BigUint operator-(const BigUint& lhs, const BigUint& rhs) {
    BigUint tmp = lhs;
    tmp -= rhs;
    return tmp;
  }

  friend constexpr BigUint MultiplyNaive(const BigUint& lhs, const BigUint& rhs) {
    std::vector<uint64_t> ans(lhs.size() + rhs.size());
    for (std::size_t i = 0; i < lhs.size(); ++i) {
      for (std::size_t j = 0; j < rhs.size(); ++j) {
        uint128_t carry = static_cast<uint128_t>(lhs[i]) * static_cast<uint128_t>(rhs[j]);
        for (std::size_t k = i + j; carry > 0; ++k) {
          const auto mul = static_cast<uint128_t>(ans[k]) + carry;
          ans[k] = static_cast<uint64_t>(mul);
          carry = mul >> 64;
        }
      }
    }

    BigUint ret = BigUint(std::move(ans));
    ret.TrimLeadingZeros();
    return ret;
  }

  friend constexpr BigUint MultiplyKaratsuba(const BigUint& lhs, const BigUint& rhs) {
    const auto max_byte_len = std::max(lhs.size(), rhs.size());
    const auto min_byte_len = std::min(lhs.size(), rhs.size());

    if (min_byte_len <= 64) {
      return MultiplyNaive(lhs, rhs);
    }

    const auto shift_bits = (max_byte_len + 1) / 2 * 64;
    const auto lhs_high = lhs >> shift_bits;
    const auto rhs_high = rhs >> shift_bits;
    const auto lhs_low = lhs.ShiftMod2Pow(0, shift_bits);
    const auto rhs_low = rhs.ShiftMod2Pow(0, shift_bits);

    const auto k1 = MultiplyKaratsuba(lhs_low, rhs_low);
    const auto k2 = MultiplyKaratsuba(lhs_high, rhs_high);
    const auto k3 = MultiplyKaratsuba(lhs_high + lhs_low, rhs_high + rhs_low);

    BigUint result = k1;
    result.ShlAddAssign(k2, 2 * shift_bits);
    result.ShlAddAssign(k3 - k1 - k2, shift_bits);
    return result;
  }

  friend constexpr BigUint operator*(const BigUint& lhs, const BigUint& rhs) {
    const auto min_byte_len = std::min(lhs.size(), rhs.size());

    if (min_byte_len <= 64) {
      return MultiplyNaive(lhs, rhs);
    } else {
      return MultiplyKaratsuba(lhs, rhs);
    }
  }

  friend constexpr BigUint operator>>(const BigUint& lhs, const std::size_t& rhs) {
    // Don't use operator>>= because it requires the whole copy of `lhs`
    const auto word_idx = rhs / 64;
    const auto bit_idx = rhs % 64;

    std::vector<uint64_t> ans;
    if (word_idx < lhs.size()) {
      for (std::size_t i = 0; i < lhs.size() - word_idx; ++i) {
        auto word = lhs[i + word_idx] >> bit_idx;
        if (bit_idx > 0 && i + word_idx + 1 < lhs.size()) {
          word |= lhs[i + word_idx + 1] << (64 - bit_idx);
        }

        ans.push_back(word);
      }
    }

    BigUint ret = BigUint(std::move(ans));
    ret.TrimLeadingZeros();
    return ret;
  }

  friend constexpr BigUint operator<<(const BigUint& lhs, const std::size_t& rhs) {
    BigUint tmp = lhs;
    tmp <<= rhs;
    return tmp;
  }

  friend constexpr std::strong_ordering operator<=>(const BigUint& lhs, const BigUint& rhs) noexcept {
    if (lhs.size() != rhs.size()) {
      return lhs.size() <=> rhs.size();
    } else {
      const auto len = lhs.size();
      for (std::size_t i = 0; i < len; ++i) {
        const auto lhs_value = lhs[len - i - 1];
        const auto rhs_value = rhs[len - i - 1];
        if (lhs_value != rhs_value) {
          return lhs_value <=> rhs_value;
        }
      }

      return std::strong_ordering::equal;
    }
  }

  friend constexpr bool operator==(const BigUint& lhs, const BigUint& rhs) noexcept = default;
  // </Operators>

  // <Minor Methods>
  // The functions in this section is optimized arithmetic operations for the particular use cases, so they may not be
  // so useful for general purpose.

  /**
   * @brief *this = *this % (2 ** n)
   * @param n Index of power of 2
   */
  constexpr BigUint& ModAssign2Pow(std::size_t n) {
    const auto word_idx = n / 64;
    const auto bit_idx = n % 64;

    if (this->size() <= word_idx) {
      return *this;
    }

    this->resize(word_idx + 1);
    this->back() &= (uint64_t{1} << bit_idx) - 1;
    TrimLeadingZeros();

    return *this;
  }

  /**
   * @brief *this += 2 ** n
   * @param n Index of power of 2
   */
  constexpr BigUint& AddAssign2Pow(std::size_t n) {
    const auto word_idx = n / 64;
    const auto bit_idx = n % 64;
    if (word_idx >= this->size()) {
      // Extend if (1 << n) is out of range
      this->resize(word_idx + 1);
    }

    uint128_t carry = uint128_t{1} << bit_idx;
    for (std::size_t i = word_idx; i < this->size() && carry > 0; ++i) {
      const uint128_t sum = static_cast<uint128_t>((*this)[i]) + carry;
      (*this)[i] = static_cast<uint64_t>(sum);
      carry = sum >> 64;
    }

    if (carry > 0) {
      this->push_back(carry);
    }

    return *this;
  }

  /**
   * @brief *this += value << shift
   * @param value
   * @param shift
   * @return
   */
  constexpr BigUint& ShlAddAssign(const BigUint& rhs, const std::size_t shift) {
    const auto word_idx = shift / 64;
    const auto bit_idx = shift % 64;

    uint128_t carry = 0;
    for (std::size_t i = 0; i < rhs.size() || carry > 0; ++i) {
      uint128_t lhs_value = 0;
      if (i + word_idx < this->size()) {
        lhs_value = static_cast<uint128_t>((*this)[word_idx + i]);
      }
      uint128_t rhs_value = 0;
      if (i < rhs.size()) {
        rhs_value = static_cast<uint128_t>(rhs[i]) << bit_idx;
      }

      const auto sum = carry + static_cast<uint128_t>(lhs_value) + static_cast<uint128_t>(rhs_value);
      if (i + word_idx >= this->size()) {
        this->resize(i + word_idx + 1);
      }
      (*this)[i + word_idx] = static_cast<uint64_t>(sum);
      carry = sum >> 64;
    }

    TrimLeadingZeros();
    return *this;
  }

  /**
   * @brief (*this >> shift) % (2 ** mod)
   */
  constexpr BigUint ShiftMod2Pow(std::size_t shift, std::size_t mod) const {
    const auto shift_word_idx = shift / 64;
    const auto shift_bit_idx = shift % 64;
    const auto mod_word_idx = mod / 64;
    const auto mod_bit_idx = mod % 64;

    if (this->size() < shift_word_idx) {
      return BigUint{};
    }

    std::vector<uint64_t> ans;
    ans.reserve(mod_word_idx + 1);
    for (std::size_t i = 0; i < mod_word_idx + 1; ++i) {
      const auto src_lower = i + shift_word_idx;
      const auto src_upper = src_lower + 1;
      if (src_lower < this->size()) {
        uint128_t word = static_cast<uint128_t>((*this)[src_lower]);
        if (src_upper < this->size()) {
          word |= static_cast<uint128_t>((*this)[src_upper]) << 64;
        }
        ans.push_back(static_cast<uint64_t>(word >> shift_bit_idx));
      } else {
        break;
      }
    }

    if (ans.size() == mod_word_idx + 1) {
      if (mod_bit_idx > 0) {
        ans[mod_word_idx] &= (uint64_t{1} << mod_bit_idx) - 1;
      } else {
        ans.pop_back();
      }
    }

    BigUint ret(std::move(ans));
    ret.TrimLeadingZeros();
    return ret;
  }

  // </Minor Methods>

 private:
  constexpr BigUint& TrimLeadingZeros() noexcept {
    while (!this->empty() && this->back() == 0) {
      this->pop_back();
    }

    return *this;
  }
};
}  // namespace komori

#endif  // KOMORI_BIGUINT_HPP_