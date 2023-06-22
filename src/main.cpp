#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <tuple>

#include "bigfloat.hpp"
#include "bigint.hpp"
#include "biguint.hpp"
#include "io.hpp"
#include "ssa.hpp"

using komori::BigFloat;
using komori::BigInt;
using komori::BigUint;
using komori::Sign;

namespace {
constexpr uint64_t A = 13591409;
constexpr uint64_t B = 545140134;
constexpr uint64_t C = 640320;
constexpr uint64_t C3 = C * C * C;

constexpr BigInt ComputeA(uint64_t n) {
  auto value = BigUint{A} + BigUint{B} * BigUint{n};
  const auto sign = (n % 2 == 0) ? Sign::kPositive : Sign::kNegative;
  return BigInt(value, sign);
}

constexpr BigInt ComputeP(uint64_t n) {
  return BigInt{2 * n - 1} * BigInt{6 * n - 5} * BigInt{6 * n - 1};
}

constexpr BigInt ComputeQ(uint64_t n) {
  return BigInt{BigUint{n}.Pow(3) * BigUint{C3 / 24}};
}

constexpr std::tuple<BigInt, BigInt, BigInt> ComputePQT(uint64_t n1, uint64_t n2) {
  if (n1 + 1 == n2) {
    auto p = ComputeP(n2);
    auto q = ComputeQ(n2);
    auto a = ComputeA(n2);
    auto t = Multiply(std::move(a), p);

    return {std::move(p), std::move(q), std::move(t)};
  } else {
    const auto m = (n1 + n2) / 2;

    auto [p1, q1, t1] = ComputePQT(n1, m);
    auto [p2, q2, t2] = ComputePQT(m, n2);

    auto t1_tmp = Multiply(std::move(t1), q2);
    auto t2_tmp = Multiply(std::move(t2), p1);
    auto t = std::move(t1_tmp) + std::move(t2_tmp);

    auto p = Multiply(std::move(p1), std::move(p2));
    auto q = Multiply(std::move(q1), std::move(q2));

    return {std::move(p), std::move(q), std::move(t)};
  }
}

constexpr BigFloat ComputePi(uint64_t digit_len) {
  constexpr double log2_10 = 3.321928094887362;

  const auto n = std::max<uint64_t>(digit_len / 14, 1);
  const auto precision = static_cast<uint64_t>(static_cast<double>(digit_len) * log2_10) + 1;
  auto [p, q, t] = ComputePQT(0, n);
  auto sqrt_c_inv = SqrtInverse(BigFloat(precision, BigInt{C}));

  auto numerator = BigFloat(precision, Multiply(BigInt{C * C}, q));
  BigFloat denominator = BigFloat(precision, Multiply(BigInt{12}, (Multiply(BigInt{A}, std::move(q)) + std::move(t))));

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
