#pragma once

#include <cstddef>
#include <vector>
#include <stdexcept>
#include <utility>
#include "operation_table.hpp"
#include "laws.hpp"

namespace discretex::algebra {

// Represents a finite group: an algebraic structure (G, *) with an associative
// binary operation, identity element e, and inverse elements for all elements.
template <FiniteDomain Dom = index_domain>
class finite_group {
public:
    using domain_type = Dom;

    explicit finite_group(Dom domain, operation_table<Dom> op)
        : domain_(std::move(domain)), op_(std::move(op)) {
        if (!is_associative(op_)) {
            throw std::invalid_argument("Operation table violates associativity; cannot construct finite_group");
        }
        auto id = find_identity(op_);
        if (!id.has_value()) {
            throw std::invalid_argument("Operation table lacks an identity element; cannot construct finite_group");
        }
        identity_ = *id;
        auto inv = inverse_table(op_, identity_);
        if (!inv.has_value()) {
            throw std::invalid_argument("Not all elements possess two-sided inverses; cannot construct finite_group");
        }
        inverses_ = std::move(*inv);
    }

    template <typename F>
    static finite_group from_callable(Dom domain, F&& op) {
        auto table = operation_table<Dom>::from_callable(domain, std::forward<F>(op));
        return finite_group(std::move(domain), std::move(table));
    }

    [[nodiscard]] std::size_t size() const noexcept { return domain_.size(); }
    [[nodiscard]] std::size_t order() const noexcept { return domain_.size(); }
    [[nodiscard]] const Dom& domain() const noexcept { return domain_; }
    [[nodiscard]] std::size_t identity() const noexcept { return identity_; }
    [[nodiscard]] std::size_t op(std::size_t a, std::size_t b) const noexcept { return op_(a, b); }
    [[nodiscard]] std::size_t operator()(std::size_t a, std::size_t b) const noexcept { return op_(a, b); }

    [[nodiscard]] std::size_t inverse(std::size_t a) const {
        if (a >= inverses_.size()) {
            throw std::out_of_range("Element index out of bounds in finite_group::inverse");
        }
        return inverses_[a];
    }

    [[nodiscard]] const std::vector<std::size_t>& inverses() const noexcept { return inverses_; }
    [[nodiscard]] const operation_table<Dom>& operation() const noexcept { return op_; }
    [[nodiscard]] bool is_abelian() const noexcept { return discretex::algebra::is_commutative(op_); }

    // Computes the order of element g in the group: smallest k >= 1 such that g^k = e.
    [[nodiscard]] std::size_t element_order(std::size_t g) const {
        if (g >= size()) {
            throw std::out_of_range("Element index out of bounds in finite_group::element_order");
        }
        std::size_t curr = g;
        std::size_t ord = 1;
        while (curr != identity_) {
            curr = op_(curr, g);
            ++ord;
            if (ord > size()) {
                throw std::runtime_error("Exceeded group order during element order calculation");
            }
        }
        return ord;
    }

    // Checks if a subset of element indices forms a subgroup.
    // 1. Non-empty and contains identity.
    // 2. Closed under group operation.
    // 3. Closed under inverses.
    [[nodiscard]] bool is_subgroup(const std::vector<std::size_t>& subset) const {
        if (subset.empty()) return false;
        std::vector<bool> in_h(size(), false);
        for (std::size_t x : subset) {
            if (x >= size()) return false;
            in_h[x] = true;
        }
        if (!in_h[identity_]) return false;
        for (std::size_t x : subset) {
            if (!in_h[inverse(x)]) return false;
            for (std::size_t y : subset) {
                if (!in_h[op_(x, y)]) return false;
            }
        }
        return true;
    }

private:
    Dom domain_;
    operation_table<Dom> op_;
    std::size_t identity_{0};
    std::vector<std::size_t> inverses_;
};

} // namespace discretex::algebra
