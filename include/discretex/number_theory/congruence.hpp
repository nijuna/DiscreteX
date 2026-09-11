#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include <utility>
#include "modular.hpp"
#include "euclidean.hpp"

namespace discretex::number_theory {

// Solves a * x = b (mod m)
// Returns all solutions in [0, m) in ascending order
inline std::vector<uint64_t> solve_linear_congruence(uint64_t a, uint64_t b, uint64_t m) {
    if (m <= 1) return {};
    a %= m;
    b %= m;

    auto eg = extended_gcd(static_cast<int64_t>(a), static_cast<int64_t>(m));
    uint64_t g = static_cast<uint64_t>(eg.gcd);

    if (g == 0 || (b % g) != 0) {
        return {}; // No solution
    }

    // Particular solution: x0 = (eg.x * (b / g)) mod (m / g)
    int64_t mod_reduced = static_cast<int64_t>(m / g);
    int64_t b_reduced = static_cast<int64_t>(b / g);
#if defined(__SIZEOF_INT128__)
    int64_t x0 = static_cast<int64_t>((static_cast<int128_t>(eg.x) * b_reduced) % mod_reduced);
#else
    int64_t x0 = (eg.x * b_reduced) % mod_reduced;
#endif
    if (x0 < 0) x0 += mod_reduced;

    std::vector<uint64_t> solutions;
    solutions.reserve(g);
    uint64_t step = m / g;
    for (uint64_t i = 0; i < g; ++i) {
        solutions.push_back((static_cast<uint64_t>(x0) + i * step) % m);
    }
    return solutions;
}

// System congruence entry: x = remainder (mod modulus)
struct congruence {
    uint64_t remainder{0};
    uint64_t modulus{1};

    bool operator==(const congruence& o) const = default;
};

// General Chinese Remainder Theorem (CRT)
// Solves system x = r_i (mod m_i) for i = 0..k-1
// Works for both pairwise coprime and non-coprime moduli.
// Returns {x0, M} where x = x0 (mod M), or std::nullopt if system is inconsistent.
inline std::optional<std::pair<uint64_t, uint64_t>>
chinese_remainder_theorem(const std::vector<congruence>& system) {
    if (system.empty()) return std::nullopt;

    uint64_t x0 = system[0].remainder % system[0].modulus;
    uint64_t m0 = system[0].modulus;
    if (m0 == 0) return std::nullopt;

    for (std::size_t i = 1; i < system.size(); ++i) {
        uint64_t r1 = system[i].remainder % system[i].modulus;
        uint64_t m1 = system[i].modulus;
        if (m1 == 0) return std::nullopt;

        auto eg = extended_gcd(static_cast<int64_t>(m0), static_cast<int64_t>(m1));
        uint64_t g = static_cast<uint64_t>(eg.gcd);

        int64_t diff = static_cast<int64_t>(r1) - static_cast<int64_t>(x0);
        if (diff % static_cast<int64_t>(g) != 0) {
            return std::nullopt; // Inconsistent system
        }

        uint64_t m1_prime = m1 / g;
        int64_t factor = diff / static_cast<int64_t>(g);
        int64_t inv = eg.x % static_cast<int64_t>(m1_prime);
        if (inv < 0) inv += static_cast<int64_t>(m1_prime);

#if defined(__SIZEOF_INT128__)
        int64_t t = static_cast<int64_t>((static_cast<int128_t>(factor % static_cast<int64_t>(m1_prime)) * inv) % static_cast<int64_t>(m1_prime));
#else
        int64_t t = ((factor % static_cast<int64_t>(m1_prime)) * inv) % static_cast<int64_t>(m1_prime);
#endif
        if (t < 0) t += static_cast<int64_t>(m1_prime);

        uint64_t new_m = m0 * m1_prime;
        uint64_t new_x = (x0 + mul_mod(static_cast<uint64_t>(t), m0, new_m)) % new_m;

        x0 = new_x;
        m0 = new_m;
    }

    return std::make_pair(x0, m0);
}

} // namespace discretex::number_theory
