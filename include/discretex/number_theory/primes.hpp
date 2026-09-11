#pragma once

#include <cstdint>
#include <vector>
#include <utility>
#include <algorithm>
#include <numeric>
#include "modular.hpp"
#include "euclidean.hpp"

namespace discretex::number_theory {

// Deterministic Miller-Rabin Primality Test for all 64-bit integers
inline bool is_prime(uint64_t n) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if ((n & 1) == 0 || n % 3 == 0) return false;

    // Small primes optimization
    if (n < 25) return true;

    // Write n - 1 as d * 2^s
    uint64_t d = n - 1;
    unsigned int s = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++s;
    }

    // Deterministic base set for all n < 2^64
    // {2, 325, 9375, 28178, 450775, 9780504, 1795265022}
    static const uint64_t bases[] = {
        2, 325, 9375, 28178, 450775, 9780504, 1795265022ULL
    };

    for (uint64_t a : bases) {
        if (a % n == 0) continue;
        uint64_t x = power_mod(a, d, n);
        if (x == 1 || x == n - 1) continue;

        bool composite = true;
        for (unsigned int r = 1; r < s; ++r) {
            x = mul_mod(x, x, n);
            if (x == n - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

// Sieve of Eratosthenes up to limit
inline std::vector<uint64_t> sieve_of_eratosthenes(uint64_t limit) {
    if (limit < 2) return {};
    std::vector<bool> is_p(limit + 1, true);
    is_p[0] = false;
    is_p[1] = false;

    for (uint64_t p = 2; p * p <= limit; ++p) {
        if (is_p[p]) {
            for (uint64_t multiple = p * p; multiple <= limit; multiple += p) {
                is_p[multiple] = false;
            }
        }
    }

    std::vector<uint64_t> primes;
    for (uint64_t i = 2; i <= limit; ++i) {
        if (is_p[i]) {
            primes.push_back(i);
        }
    }
    return primes;
}

// Canonical prime factorization: returns list of (prime, exponent) pairs
inline std::vector<std::pair<uint64_t, std::size_t>> prime_factors(uint64_t n) {
    if (n < 2) return {};
    std::vector<std::pair<uint64_t, std::size_t>> factors;

    // Extract 2
    if ((n & 1) == 0) {
        std::size_t count = 0;
        while ((n & 1) == 0) {
            ++count;
            n >>= 1;
        }
        factors.emplace_back(2, count);
    }

    // Extract 3
    if (n % 3 == 0) {
        std::size_t count = 0;
        while (n % 3 == 0) {
            ++count;
            n /= 3;
        }
        factors.emplace_back(3, count);
    }

    // Extract 6k +/- 1
    for (uint64_t d = 5; d * d <= n; d += 6) {
        if (n % d == 0) {
            std::size_t count = 0;
            while (n % d == 0) {
                ++count;
                n /= d;
            }
            factors.emplace_back(d, count);
        }
        uint64_t d2 = d + 2;
        if (n % d2 == 0) {
            std::size_t count = 0;
            while (n % d2 == 0) {
                ++count;
                n /= d2;
            }
            factors.emplace_back(d2, count);
        }
    }

    if (n > 1) {
        factors.emplace_back(n, 1);
    }
    return factors;
}

namespace detail {
inline void generate_divisors_dfs(std::size_t idx,
                                  uint64_t current,
                                  const std::vector<std::pair<uint64_t, std::size_t>>& factors,
                                  std::vector<uint64_t>& out) {
    if (idx == factors.size()) {
        out.push_back(current);
        return;
    }
    uint64_t p = factors[idx].first;
    std::size_t exp = factors[idx].second;

    uint64_t p_pow = 1;
    for (std::size_t e = 0; e <= exp; ++e) {
        generate_divisors_dfs(idx + 1, current * p_pow, factors, out);
        if (e < exp) p_pow *= p;
    }
}
} // namespace detail

// Returns all positive divisors of n in sorted order
inline std::vector<uint64_t> divisors(uint64_t n) {
    if (n == 0) return {};
    if (n == 1) return {1};

    auto factors = prime_factors(n);
    std::vector<uint64_t> divs;
    detail::generate_divisors_dfs(0, 1, factors, divs);
    std::sort(divs.begin(), divs.end());
    return divs;
}

// Euler Totient Function: phi(n)
inline uint64_t euler_totient(uint64_t n) {
    if (n == 0) return 0;
    if (n == 1) return 1;

    auto factors = prime_factors(n);
    uint64_t result = n;
    for (const auto& [p, _] : factors) {
        result = (result / p) * (p - 1);
    }
    return result;
}

// Carmichael Function: lambda(n)
inline uint64_t carmichael(uint64_t n) {
    if (n <= 2) return 1;
    if (n == 4) return 2;

    auto factors = prime_factors(n);
    uint64_t lambda_val = 1;

    for (const auto& [p, a] : factors) {
        uint64_t pk_lambda = 1;
        if (p == 2) {
            if (a == 1) pk_lambda = 1;
            else if (a == 2) pk_lambda = 2;
            else {
                // 2^(a-2)
                pk_lambda = 1ULL << (a - 2);
            }
        } else {
            // p^(a-1) * (p - 1)
            uint64_t p_pow = 1;
            for (std::size_t i = 1; i < a; ++i) p_pow *= p;
            pk_lambda = p_pow * (p - 1);
        }
        lambda_val = std::lcm(lambda_val, pk_lambda);
    }
    return lambda_val;
}

} // namespace discretex::number_theory
