#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <tuple>

#include "komori/bigfloat.hpp"
#include "komori/biguint.hpp"
#include "komori/io.hpp"
#include "komori/ssa.hpp"

using komori::BigFloat;
using komori::BigUint;

namespace {
constexpr uint64_t A = 13591409;
constexpr uint64_t B = 545140134;
constexpr uint64_t C = 640320;
constexpr uint64_t C3 = C * C * C;

constexpr std::pair<BigUint, bool> ComputeA(uint64_t n) {
  return std::make_pair(BigUint{A} + BigUint{B} * BigUint{n}, n % 2 == 1);
}

constexpr BigUint ComputeP(uint64_t n) {
  return BigUint{2 * n - 1} * BigUint{6 * n - 5} * BigUint{6 * n - 1};
}

constexpr BigUint ComputeQ(uint64_t n) {
  return BigUint{n}.Pow(3) * BigUint{C3 / 24};
}

constexpr std::tuple<BigUint, BigUint, std::pair<BigUint, bool>> ComputePQT(uint64_t n1, uint64_t n2) {
  if (n1 + 1 == n2) {
    auto p = ComputeP(n2);
    auto q = ComputeQ(n2);
    auto [a, is_negative] = ComputeA(n2);
    auto t = Multiply(std::move(a), p);

    return std::make_tuple(std::move(p), std::move(q), std::make_pair(std::move(t), is_negative));
  } else {
    const auto m = (n1 + n2) / 2;

    auto [p1, q1, t1] = ComputePQT(n1, m);
    auto [p2, q2, t2] = ComputePQT(m, n2);

    auto t1_tmp = Multiply(std::move(t1.first), q2);
    auto t2_tmp = Multiply(std::move(t2.first), p1);

    bool is_negative = false;
    BigUint t{};
    if (t1.second == t2.second) {
      t = std::move(t1_tmp) + std::move(t2_tmp);
      is_negative = t1.second;
    } else if (t1_tmp < t2_tmp) {
      t = std::move(t2_tmp) - std::move(t1_tmp);
      is_negative = t2.second;
    } else {
      t = std::move(t1_tmp) - std::move(t2_tmp);
      is_negative = t1.second;
    }

    auto p = Multiply(std::move(p1), std::move(p2));
    auto q = Multiply(std::move(q1), std::move(q2));

    return std::make_tuple(std::move(p), std::move(q), std::make_pair(std::move(t), is_negative));
  }
}

constexpr BigFloat ComputePi(uint64_t digit_len) {
  constexpr double log2_10 = 3.321928094887362;

  const auto n = std::max<uint64_t>(digit_len / 14, 1);
  const auto precision = static_cast<uint64_t>(static_cast<double>(digit_len) * log2_10) + 1;
  auto [p, q, t] = ComputePQT(0, n);
  auto sqrt_c_inv = SqrtInverse(BigFloat(precision, BigUint{C}));

  auto numerator = BigFloat(precision, Multiply(BigUint{C * C}, q));
  BigFloat denominator(precision);
  if (!t.second) {
    denominator = BigFloat(precision, Multiply(BigUint{12}, (Multiply(BigUint{A}, std::move(q)) + std::move(t.first))));
  } else {
    denominator = BigFloat(precision, Multiply(BigUint{12}, (Multiply(BigUint{A}, std::move(q)) - std::move(t.first))));
  }

  return numerator * sqrt_c_inv / denominator;
}

template <std::size_t N>
std::array<char, N + 3> GetPiString() {
  auto str = ToString(ComputePi(N + 2)).substr(0, N + 2);

  std::array<char, N + 3> ans{};
  std::copy(str.begin(), str.end(), ans.begin());
  return ans;
}

template <std::size_t N>
constexpr std::size_t TestMultiply() {
  std::vector<uint64_t> x(N, 0x334);
  std::vector<uint64_t> y(N, 0x264);

  return MultiplyNaive(BigUint(std::move(x)), BigUint(std::move(y))).size();
  // return MultiplyKaratsuba(BigUint(std::move(x)), BigUint(std::move(y))).size();
  // return komori::detail::MultiplySSA(BigUint(std::move(x)), BigUint(std::move(y))).size();
}
}  // namespace

int main() {
  auto ans = GetPiString<100000>();
  std::cout << ans.data() << std::endl;
  // constexpr auto ans = TestMultiply<128>();
  // std::cout << ans << std::endl;

  return EXIT_SUCCESS;
}
