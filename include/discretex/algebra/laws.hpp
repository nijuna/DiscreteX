#pragma once

#include <cstddef>
#include <vector>
#include <optional>
#include "operation_table.hpp"

namespace discretex::algebra {

// Checks associativity: (a * b) * c == a * (b * c) for all a, b, c in carrier.
// Complexity: O(n^3).
template <FiniteDomain Dom>
bool is_associative(const operation_table<Dom>& op) noexcept {
    std::size_t n = op.size();
    for (std::size_t a = 0; a < n; ++a) {
        for (std::size_t b = 0; b < n; ++b) {
            std::size_t ab = op(a, b);
            for (std::size_t c = 0; c < n; ++c) {
                std::size_t bc = op(b, c);
                if (op(ab, c) != op(a, bc)) {
                    return false;
                }
            }
        }
    }
    return true;
}

// Checks commutativity: a * b == b * a for all a, b in carrier.
// Complexity: O(n^2).
template <FiniteDomain Dom>
bool is_commutative(const operation_table<Dom>& op) noexcept {
    std::size_t n = op.size();
    for (std::size_t a = 0; a < n; ++a) {
        for (std::size_t b = a + 1; b < n; ++b) {
            if (op(a, b) != op(b, a)) {
                return false;
            }
        }
    }
    return true;
}

// Finds the unique two-sided identity element e such that e * a == a and a * e == a for all a.
// Returns std::nullopt if no two-sided identity exists.
// Complexity: O(n^2).
template <FiniteDomain Dom>
std::optional<std::size_t> find_identity(const operation_table<Dom>& op) noexcept {
    std::size_t n = op.size();
    for (std::size_t e = 0; e < n; ++e) {
        bool is_id = true;
        for (std::size_t a = 0; a < n; ++a) {
            if (op(e, a) != a || op(a, e) != a) {
                is_id = false;
                break;
            }
        }
        if (is_id) {
            return e;
        }
    }
    return std::nullopt;
}

// Returns true if a two-sided identity element exists.
template <FiniteDomain Dom>
bool has_identity(const operation_table<Dom>& op) noexcept {
    return find_identity(op).has_value();
}

// Computes the inverse table mapping each element a to its two-sided inverse a^-1.
// Returns std::nullopt if any element lacks a two-sided inverse.
// Complexity: O(n^2).
template <FiniteDomain Dom>
std::optional<std::vector<std::size_t>> inverse_table(
    const operation_table<Dom>& op, std::size_t identity) noexcept {
    std::size_t n = op.size();
    if (identity >= n) return std::nullopt;

    std::vector<std::size_t> inv(n, n);
    for (std::size_t a = 0; a < n; ++a) {
        bool found = false;
        for (std::size_t b = 0; b < n; ++b) {
            if (op(a, b) == identity && op(b, a) == identity) {
                inv[a] = b;
                found = true;
                break;
            }
        }
        if (!found) {
            return std::nullopt;
        }
    }
    return inv;
}

// Returns true if every element in the carrier has a two-sided inverse.
template <FiniteDomain Dom>
bool has_inverses(const operation_table<Dom>& op, std::size_t identity) noexcept {
    return inverse_table(op, identity).has_value();
}

// Checks idempotence: a * a == a for all a.
template <FiniteDomain Dom>
bool is_idempotent(const operation_table<Dom>& op) noexcept {
    std::size_t n = op.size();
    for (std::size_t a = 0; a < n; ++a) {
        if (op(a, a) != a) return false;
    }
    return true;
}

// Checks left distributivity: a * (b + c) == (a * b) + (a * c).
template <FiniteDomain Dom>
bool is_left_distributive(const operation_table<Dom>& mul,
                          const operation_table<Dom>& add) noexcept {
    std::size_t n = mul.size();
    if (add.size() != n) return false;

    for (std::size_t a = 0; a < n; ++a) {
        for (std::size_t b = 0; b < n; ++b) {
            for (std::size_t c = 0; c < n; ++c) {
                std::size_t lhs = mul(a, add(b, c));
                std::size_t rhs = add(mul(a, b), mul(a, c));
                if (lhs != rhs) return false;
            }
        }
    }
    return true;
}

// Checks right distributivity: (a + b) * c == (a * c) + (b * c).
template <FiniteDomain Dom>
bool is_right_distributive(const operation_table<Dom>& mul,
                           const operation_table<Dom>& add) noexcept {
    std::size_t n = mul.size();
    if (add.size() != n) return false;

    for (std::size_t a = 0; a < n; ++a) {
        for (std::size_t b = 0; b < n; ++b) {
            for (std::size_t c = 0; c < n; ++c) {
                std::size_t lhs = mul(add(a, b), c);
                std::size_t rhs = add(mul(a, c), mul(b, c));
                if (lhs != rhs) return false;
            }
        }
    }
    return true;
}

// Checks two-sided distributivity of multiplication over addition.
template <FiniteDomain Dom>
bool is_distributive(const operation_table<Dom>& mul,
                     const operation_table<Dom>& add) noexcept {
    return is_left_distributive(mul, add) && is_right_distributive(mul, add);
}

// Checks lattice absorption laws:
// 1. a \/ (a /\ b) == a
// 2. a /\ (a \/ b) == a
template <FiniteDomain Dom>
bool is_absorptive(const operation_table<Dom>& join_op,
                   const operation_table<Dom>& meet_op) noexcept {
    std::size_t n = join_op.size();
    if (meet_op.size() != n) return false;

    for (std::size_t a = 0; a < n; ++a) {
        for (std::size_t b = 0; b < n; ++b) {
            if (join_op(a, meet_op(a, b)) != a) return false;
            if (meet_op(a, join_op(a, b)) != a) return false;
        }
    }
    return true;
}

// Checks Boolean complement laws:
// 1. a \/ comp(a) == top
// 2. a /\ comp(a) == bottom
template <FiniteDomain Dom>
bool satisfies_boolean_complements(const operation_table<Dom>& join_op,
                                  const operation_table<Dom>& meet_op,
                                  const std::vector<std::size_t>& complement,
                                  std::size_t bottom,
                                  std::size_t top) noexcept {
    std::size_t n = join_op.size();
    if (meet_op.size() != n || complement.size() != n) return false;

    for (std::size_t a = 0; a < n; ++a) {
        std::size_t not_a = complement[a];
        if (not_a >= n) return false;
        if (join_op(a, not_a) != top) return false;
        if (meet_op(a, not_a) != bottom) return false;
    }
    return true;
}

} // namespace discretex::algebra
