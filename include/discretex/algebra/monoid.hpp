#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include "operation_table.hpp"
#include "laws.hpp"

namespace discretex::algebra {

// Represents a finite monoid: an algebraic structure (M, *) with an associative
// binary operation and a two-sided identity element e.
template <FiniteDomain Dom = index_domain>
class finite_monoid {
public:
    using domain_type = Dom;

    explicit finite_monoid(Dom domain, operation_table<Dom> op)
        : domain_(std::move(domain)), op_(std::move(op)) {
        if (!is_associative(op_)) {
            throw std::invalid_argument("Operation table violates associativity; cannot construct finite_monoid");
        }
        auto id = find_identity(op_);
        if (!id.has_value()) {
            throw std::invalid_argument("Operation table lacks a two-sided identity element; cannot construct finite_monoid");
        }
        identity_ = *id;
    }

    template <typename F>
    static finite_monoid from_callable(Dom domain, F&& op) {
        auto table = operation_table<Dom>::from_callable(domain, std::forward<F>(op));
        return finite_monoid(std::move(domain), std::move(table));
    }

    [[nodiscard]] std::size_t size() const noexcept { return domain_.size(); }
    [[nodiscard]] const Dom& domain() const noexcept { return domain_; }
    [[nodiscard]] std::size_t identity() const noexcept { return identity_; }
    [[nodiscard]] std::size_t op(std::size_t a, std::size_t b) const noexcept { return op_(a, b); }
    [[nodiscard]] std::size_t operator()(std::size_t a, std::size_t b) const noexcept { return op_(a, b); }
    [[nodiscard]] const operation_table<Dom>& operation() const noexcept { return op_; }
    [[nodiscard]] bool is_commutative() const noexcept { return discretex::algebra::is_commutative(op_); }

private:
    Dom domain_;
    operation_table<Dom> op_;
    std::size_t identity_{0};
};

} // namespace discretex::algebra
