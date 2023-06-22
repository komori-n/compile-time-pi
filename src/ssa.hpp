#ifndef KOMORI_SSA_HPP_
#define KOMORI_SSA_HPP_

#include <iostream>

#include "biguint.hpp"
#include "gf2n1.hpp"

namespace komori {
namespace detail {
constexpr inline uint64_t Calc_n(uint64_t k) noexcept {
  return (1 << (k - 1));
}

constexpr inline uint64_t Calc_M(uint64_t k) noexcept {
  return (Calc_n(k) - k) / 2;
}

constexpr inline uint64_t Best_k(uint64_t bit_len) noexcept {
  uint64_t l = 0;
  uint64_t r = 32;
  while (r - l > 1) {
    const auto m = (l + r) / 2;
    const auto max_len = Calc_M(m) * (uint64_t{1} << (m - 1));
    if (max_len >= bit_len) {
      r = m;
    } else {
      l = m;
    }
  }

  return r;
}

class SplittedInteger {
 public:
  explicit constexpr SplittedInteger(const BigUint& num, uint64_t k) : k_{k}, n_{Calc_n(k)}, m_{Calc_M(k)} {
    const auto N = uint64_t{1} << k;
    values_.reserve(N);
    for (uint64_t i = 0; i < N; ++i) {
      BigUint x = num.ShiftMod2Pow(i * m_, m_);
      values_.emplace_back(n_, std::move(x));
    }
  }

  constexpr BigUint Get() const noexcept {
    const auto N = uint64_t{1} << k_;
    BigUint ans{};
    for (uint64_t i = 0; i < N; ++i) {
      const auto& x = values_[i].Get();
      ans.ShlAddAssign(x, i * m_);
    }

    return ans;
  }

  constexpr void NTT() noexcept {
    const uint64_t len = values_.size();
    for (uint64_t q = len / 2; q > 0; q /= 2) {
      const auto p = len / q / 2;
      for (uint64_t i = 0; i < q; ++i) {
        const auto w = GF2PowNPlus1::Make2Pow(i * p, n_);
        for (uint64_t j = i; j < len; j += 2 * q) {
          const auto k = j + q;
          auto tmp = values_[j] - values_[k];
          values_[j] += values_[k];
          tmp *= w;
          values_[k] = std::move(tmp);
        }
      }
    }

    uint64_t i = 0;
    for (uint64_t j = 1; j < len; ++j) {
      auto l = len / 2;
      i ^= l;
      while (i < l) {
        l /= 2;
        i ^= l;
      }
      if (j < i) {
        std::swap(values_[i], values_[j]);
      }
    }
  }

  constexpr void INTT() noexcept {
    NTT();
    for (std::size_t i = 1; i < values_.size() / 2; ++i) {
      std::swap(values_[i], values_[values_.size() - i]);
    }

    const auto w = GF2PowNPlus1::Make2Pow(2 * n_ - k_, n_);
    for (auto& value : values_) {
      value *= w;
    }
  }

  constexpr SplittedInteger& operator*=(const SplittedInteger& rhs) {
    for (uint64_t i = 0; i < values_.size(); ++i) {
      values_[i] *= rhs.values_[i];
    }

    return *this;
  }

 private:
  std::vector<GF2PowNPlus1> values_;
  uint64_t k_;
  uint64_t n_;
  uint64_t m_;
};

constexpr inline BigUint MultiplySSA(const BigUint& lhs, const BigUint& rhs) {
  const auto bit_len = std::max(lhs.NumberOfBits(), rhs.NumberOfBits()) * 64;
  const auto best_k = Best_k(bit_len);

  SplittedInteger l(lhs, best_k);
  SplittedInteger r(rhs, best_k);
  l.NTT();
  r.NTT();
  l *= r;
  l.INTT();
  return l.Get();
}
}  // namespace detail

constexpr inline BigUint Multiply(const BigUint& lhs, const BigUint& rhs) {
  if (true) {
    return lhs * rhs;
  } else {
    // Multiplication by SSA is disabled because it requires tremendous time
    return detail::MultiplySSA(lhs, rhs);
  }
}
}  // namespace komori

#endif  // KOMORI_SSA_HPP_