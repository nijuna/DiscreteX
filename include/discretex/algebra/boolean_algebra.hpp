#pragma once

#include <cstddef>
#include <vector>
#include <stdexcept>
#include <utility>
#include "operation_table.hpp"
#include "laws.hpp"

namespace discretex::algebra {

// Represents a finite Boolean algebra (B, \/, /\, ~, bot, top) satisfying
// commutativity, associativity, absorption, distributivity, and complementation.
template <FiniteDomain Dom = index_domain>
class finite_boolean_algebra {
public:
    using domain_type = Dom;

    finite_boolean_algebra(Dom domain,
                           operation_table<Dom> join_op,
                           operation_table<Dom> meet_op,
                           std::vector<std::size_t> complement,
                           std::size_t bottom,
                           std::size_t top)
        : domain_(std::move(domain)),
          join_(std::move(join_op)),
          meet_(std::move(meet_op)),
          complement_(std::move(complement)),
          bottom_(bottom),
          top_(top) {
        std::size_t n = domain_.size();
        if (n == 0) {
            throw std::invalid_argument("Boolean algebra carrier domain must be non-empty");
        }
        if (bottom_ >= n || top_ >= n || complement_.size() != n) {
            throw std::out_of_range("Element bounds or complement vector dimension mismatch");
        }

        // 1. Commutativity and Associativity
        if (!is_commutative(join_) || !is_commutative(meet_)) {
            throw std::invalid_argument("Join and meet operations must be commutative");
        }
        if (!is_associative(join_) || !is_associative(meet_)) {
            throw std::invalid_argument("Join and meet operations must be associative");
        }

        // 2. Absorption laws
        if (!is_absorptive(join_, meet_)) {
            throw std::invalid_argument("Join and meet operations must satisfy absorption laws");
        }

        // 3. Two-sided distributivity
        if (!is_distributive(meet_, join_) || !is_distributive(join_, meet_)) {
            throw std::invalid_argument("Operations must distribute over each other");
        }

        // 4. Complement laws
        if (!satisfies_boolean_complements(join_, meet_, complement_, bottom_, top_)) {
            throw std::invalid_argument("Operations and complement vector must satisfy complement laws");
        }
    }

    [[nodiscard]] std::size_t size() const noexcept { return domain_.size(); }
    [[nodiscard]] const Dom& domain() const noexcept { return domain_; }

    [[nodiscard]] std::size_t join(std::size_t a, std::size_t b) const noexcept { return join_(a, b); }
    [[nodiscard]] std::size_t meet(std::size_t a, std::size_t b) const noexcept { return meet_(a, b); }
    [[nodiscard]] std::size_t complement(std::size_t a) const {
        if (a >= complement_.size()) throw std::out_of_range("Index out of bounds in complement");
        return complement_[a];
    }

    [[nodiscard]] std::size_t bottom() const noexcept { return bottom_; }
    [[nodiscard]] std::size_t top() const noexcept { return top_; }

    [[nodiscard]] const operation_table<Dom>& join_operation() const noexcept { return join_; }
    [[nodiscard]] const operation_table<Dom>& meet_operation() const noexcept { return meet_; }
    [[nodiscard]] const std::vector<std::size_t>& complement_vector() const noexcept { return complement_; }

private:
    Dom domain_;
    operation_table<Dom> join_;
    operation_table<Dom> meet_;
    std::vector<std::size_t> complement_;
    std::size_t bottom_{0};
    std::size_t top_{0};
};

} // namespace discretex::algebra
