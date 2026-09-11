#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <numeric>

namespace discretex::combinatorics {

namespace detail {

inline uint64_t safe_add(uint64_t a, uint64_t b, const char* msg = "Integer addition overflow") {
    uint64_t res = 0;
    if (__builtin_add_overflow(a, b, &res)) {
        throw std::overflow_error(msg);
    }
    return res;
}

inline uint64_t safe_mul(uint64_t a, uint64_t b, const char* msg = "Integer multiplication overflow") {
    uint64_t res = 0;
    if (__builtin_mul_overflow(a, b, &res)) {
        throw std::overflow_error(msg);
    }
    return res;
}

} // namespace detail

// Factorial: n!
inline uint64_t factorial(uint64_t n) {
    if (n > 20) {
        throw std::overflow_error("factorial(n) overflows 64-bit unsigned integer for n > 20");
    }
    uint64_t res = 1;
    for (uint64_t i = 2; i <= n; ++i) {
        res *= i;
    }
    return res;
}

// Falling Factorial: n^(k) = n * (n - 1) * ... * (n - k + 1)
inline uint64_t falling_factorial(uint64_t n, uint64_t k) {
    if (k > n) return 0;
    if (k == 0) return 1;

    uint64_t res = 1;
    for (uint64_t i = 0; i < k; ++i) {
        res = detail::safe_mul(res, n - i, "falling_factorial overflow");
    }
    return res;
}

// Permutations Count: P(n, k) = falling_factorial(n, k)
inline uint64_t permutations_count(uint64_t n, uint64_t k) {
    return falling_factorial(n, k);
}

// Permutations Count of n elements: n!
inline uint64_t permutations_count(uint64_t n) {
    return factorial(n);
}

// Combinations Count: C(n, k) = n! / (k! * (n - k)!)
inline uint64_t combinations_count(uint64_t n, uint64_t k) {
    if (k > n) return 0;
    if (k == 0 || k == n) return 1;
    if (k > n - k) k = n - k;

    // Use 128-bit accumulator to avoid premature intermediate overflow
    __extension__ typedef unsigned __int128 uint128_t;
    uint128_t res = 1;
    for (uint64_t i = 1; i <= k; ++i) {
        res = (res * (n - i + 1)) / i;
    }
    if (res > UINT64_MAX) {
        throw std::overflow_error("combinations_count overflow");
    }
    return static_cast<uint64_t>(res);
}

// Stirling Numbers of the Second Kind: S(n, k)
// Number of ways to partition a set of n labeled elements into k non-empty subsets.
inline uint64_t stirling_second(uint64_t n, uint64_t k) {
    if (k > n) return 0;
    if (n == 0 && k == 0) return 1;
    if (n == 0 || k == 0) return 0;
    if (k == 1 || k == n) return 1;

    // Dynamic programming using O(k) memory
    std::vector<uint64_t> dp(k + 1, 0);
    dp[0] = 0;
    dp[1] = 1;

    for (uint64_t i = 2; i <= n; ++i) {
        uint64_t max_j = std::min(i, k);
        uint64_t prev = dp[0];
        for (uint64_t j = 1; j <= max_j; ++j) {
            uint64_t current = dp[j];
            // S(i, j) = j * S(i-1, j) + S(i-1, j-1)
            uint64_t term = detail::safe_mul(j, current, "stirling_second overflow");
            dp[j] = detail::safe_add(term, prev, "stirling_second overflow");
            prev = current;
        }
        if (i <= k) {
            dp[i] = 1;
        }
    }

    return dp[k];
}

// Unsigned Stirling Numbers of the First Kind: |c(n, k)|
// Number of permutations of n elements with exactly k disjoint cycles.
inline uint64_t stirling_first_unsigned(uint64_t n, uint64_t k) {
    if (k > n) return 0;
    if (n == 0 && k == 0) return 1;
    if (n == 0 || k == 0) return 0;
    if (n == k) return 1;

    std::vector<uint64_t> dp(k + 1, 0);
    dp[1] = 1;

    for (uint64_t i = 2; i <= n; ++i) {
        uint64_t max_j = std::min(i, k);
        uint64_t prev = dp[0];
        for (uint64_t j = 1; j <= max_j; ++j) {
            uint64_t current = dp[j];
            // |c(i, j)| = (i - 1) * |c(i-1, j)| + |c(i-1, j-1)|
            uint64_t term = detail::safe_mul(i - 1, current, "stirling_first_unsigned overflow");
            dp[j] = detail::safe_add(term, prev, "stirling_first_unsigned overflow");
            prev = current;
        }
        if (i <= k) {
            dp[i] = 1;
        }
    }

    return dp[k];
}

// Alias for unsigned Stirling numbers of the first kind
inline uint64_t stirling_first(uint64_t n, uint64_t k) {
    return stirling_first_unsigned(n, k);
}

// Bell Numbers: B(n) = sum_{k=0}^n S(n, k)
// Total number of partitions of a set of n elements (and number of equivalence relations).
inline uint64_t bell_number(uint64_t n) {
    if (n == 0 || n == 1) return 1;
    if (n > 25) {
        throw std::overflow_error("bell_number(n) overflows 64-bit unsigned integer for n > 25");
    }

    // Bell Triangle (Aitken array)
    std::vector<uint64_t> prev = {1};

    for (uint64_t i = 1; i <= n; ++i) {
        std::vector<uint64_t> curr(i + 1);
        curr[0] = prev.back();
        for (uint64_t j = 1; j <= i; ++j) {
            curr[j] = detail::safe_add(curr[j - 1], prev[j - 1], "bell_number overflow");
        }
        prev = std::move(curr);
    }

    return prev[0];
}

// Set Partitions Count: alias for Bell number
inline uint64_t set_partitions_count(uint64_t n) {
    return bell_number(n);
}

// Catalan Number: C_n = (1 / (n + 1)) * (2n choose n)
inline uint64_t catalan_number(uint64_t n) {
    if (n == 0 || n == 1) return 1;
    if (n > 35) {
        throw std::overflow_error("catalan_number(n) overflows 64-bit unsigned integer for n > 35");
    }

    __extension__ typedef unsigned __int128 uint128_t;
    uint128_t c = 1;
    for (uint64_t i = 0; i < n; ++i) {
        c = (c * 2 * (2 * i + 1)) / (i + 2);
    }
    return static_cast<uint64_t>(c);
}

// Partition Number: p(n) (Unrestricted integer partitions)
// Computed via Euler pentagonal number theorem in O(n * sqrt(n))
inline uint64_t partition_number(uint64_t n) {
    if (n == 0) return 1;
    if (n > 400) {
        throw std::overflow_error("partition_number(n) overflows 64-bit unsigned integer for n > 400");
    }

    std::vector<uint64_t> p(n + 1, 0);
    p[0] = 1;

    for (uint64_t i = 1; i <= n; ++i) {
        int64_t sum = 0;
        for (int64_t k = 1;; ++k) {
            // Generalized pentagonal numbers: g_k = k * (3k - 1) / 2
            int64_t g1 = (k * (3 * k - 1)) / 2;
            int64_t g2 = (k * (3 * k + 1)) / 2;

            if (g1 > static_cast<int64_t>(i)) break;

            int64_t sign = (k % 2 != 0) ? 1 : -1;
            sum += sign * p[i - g1];

            if (g2 <= static_cast<int64_t>(i)) {
                sum += sign * p[i - g2];
            }
        }
        if (sum < 0) {
            throw std::overflow_error("partition_number overflow");
        }
        p[i] = static_cast<uint64_t>(sum);
    }

    return p[n];
}

} // namespace discretex::combinatorics
