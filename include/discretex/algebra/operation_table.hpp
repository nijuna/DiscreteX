#pragma once

#include <cstddef>
#include <vector>
#include <stdexcept>
#include <utility>
#include <concepts>
#include "../core/domain.hpp"

namespace discretex::algebra {

// Represents a binary operation over a finite carrier domain [0, n) x [0, n) -> [0, n).
// Uses a flat row-major matrix to provide O(1) table lookups.
template <FiniteDomain Dom = index_domain>
class operation_table {
public:
    using domain_type = Dom;
    using value_type = std::size_t;

    operation_table() : domain_(Dom{}), n_(0), table_() {}

    explicit operation_table(Dom domain)
        : domain_(std::move(domain)), n_(domain_.size()), table_(n_ * n_, 0) {}

    // Constructs an operation table by evaluating a callable across all pairs (a, b) in [0, n)^2.
    // Validates that the operation is closed over the domain.
    template <typename F>
        requires std::invocable<F, std::size_t, std::size_t>
    static operation_table from_callable(Dom domain, F&& op) {
        std::size_t n = domain.size();
        operation_table table(std::move(domain));

        for (std::size_t a = 0; a < n; ++a) {
            for (std::size_t b = 0; b < n; ++b) {
                std::size_t res = static_cast<std::size_t>(op(a, b));
                if (res >= n) {
                    throw std::out_of_range("Operation result out of carrier domain bounds in operation_table::from_callable");
                }
                table.table_[a * n + b] = res;
            }
        }
        return table;
    }

    // Constructs an operation table from an existing flat n*n row-major vector.
    static operation_table from_table(Dom domain, std::vector<std::size_t> table) {
        std::size_t n = domain.size();
        if (table.size() != n * n) {
            throw std::invalid_argument("Table size does not match domain dimension squared");
        }
        for (std::size_t val : table) {
            if (val >= n) {
                throw std::out_of_range("Table entry out of carrier domain bounds");
            }
        }
        operation_table t(std::move(domain));
        t.table_ = std::move(table);
        return t;
    }

    [[nodiscard]] std::size_t size() const noexcept { return n_; }
    [[nodiscard]] const Dom& domain() const noexcept { return domain_; }

    [[nodiscard]] std::size_t apply(std::size_t a, std::size_t b) const noexcept {
        return table_[a * n_ + b];
    }

    [[nodiscard]] std::size_t operator()(std::size_t a, std::size_t b) const noexcept {
        return apply(a, b);
    }

    [[nodiscard]] const std::vector<std::size_t>& raw_table() const noexcept {
        return table_;
    }

    void set(std::size_t a, std::size_t b, std::size_t val) {
        if (a >= n_ || b >= n_ || val >= n_) {
            throw std::out_of_range("Index out of bounds in operation_table::set");
        }
        table_[a * n_ + b] = val;
    }

private:
    Dom domain_;
    std::size_t n_{0};
    std::vector<std::size_t> table_;
};

} // namespace discretex::algebra
