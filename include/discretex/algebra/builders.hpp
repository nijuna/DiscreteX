#pragma once

#include <cstddef>
#include <vector>
#include <numeric>
#include <stdexcept>
#include "operation_table.hpp"
#include "group.hpp"
#include "boolean_algebra.hpp"

namespace discretex::algebra {

// Builds the additive cyclic group (Z/nZ, + mod n).
inline finite_group<index_domain> cyclic_group(std::size_t n) {
    if (n == 0) {
        throw std::invalid_argument("Cyclic group order must be strictly positive");
    }
    auto op = operation_table<index_domain>::from_callable(
        index_domain(n),
        [n](std::size_t a, std::size_t b) {
            return (a + b) % n;
        });
    return finite_group<index_domain>(index_domain(n), std::move(op));
}

// Builds the Klein four-group V_4 isomorphic to Z_2 x Z_2 under bitwise XOR.
inline finite_group<index_domain> klein_four_group() {
    auto op = operation_table<index_domain>::from_callable(
        index_domain(4),
        [](std::size_t a, std::size_t b) {
            return a ^ b;
        });
    return finite_group<index_domain>(index_domain(4), std::move(op));
}

// Builds the multiplicative unit group (Z/nZ)* of coprime residue classes modulo n.
inline finite_group<index_domain> unit_group_mod_n(std::size_t n) {
    if (n <= 1) {
        throw std::invalid_argument("Modulus for unit group must be >= 2");
    }

    std::vector<std::size_t> units;
    std::vector<std::size_t> index_of(n, n);
    for (std::size_t a = 1; a < n; ++a) {
        if (std::gcd(a, n) == 1) {
            index_of[a] = units.size();
            units.push_back(a);
        }
    }

    std::size_t phi = units.size();
    auto op = operation_table<index_domain>::from_callable(
        index_domain(phi),
        [&](std::size_t i, std::size_t j) {
            std::size_t prod = (units[i] * units[j]) % n;
            return index_of[prod];
        });

    return finite_group<index_domain>(index_domain(phi), std::move(op));
}

// Builds the canonical Boolean algebra over the power set of a k-element set (carrier size 2^k).
inline finite_boolean_algebra<index_domain> power_set_boolean_algebra(std::size_t k) {
    if (k >= 64) {
        throw std::overflow_error("Power set dimension exceeds bitmask limit");
    }
    std::size_t size = 1ULL << k;
    std::size_t mask = size - 1;

    auto join_op = operation_table<index_domain>::from_callable(
        index_domain(size),
        [](std::size_t a, std::size_t b) { return a | b; });

    auto meet_op = operation_table<index_domain>::from_callable(
        index_domain(size),
        [](std::size_t a, std::size_t b) { return a & b; });

    std::vector<std::size_t> complement(size);
    for (std::size_t a = 0; a < size; ++a) {
        complement[a] = (~a) & mask;
    }

    return finite_boolean_algebra<index_domain>(
        index_domain(size),
        std::move(join_op),
        std::move(meet_op),
        std::move(complement),
        0,
        mask);
}

// Constructs the natural projection homomorphism pi: Z_n -> Z_d where d divides n: pi(x) = x mod d.
inline std::vector<std::size_t> mod_reduction_homomorphism(std::size_t n, std::size_t d) {
    if (d == 0 || n % d != 0) {
        throw std::invalid_argument("d must be non-zero and divide n");
    }
    std::vector<std::size_t> map(n);
    for (std::size_t x = 0; x < n; ++x) {
        map[x] = x % d;
    }
    return map;
}

} // namespace discretex::algebra
