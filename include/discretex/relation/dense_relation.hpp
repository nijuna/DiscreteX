#pragma once
#include <cstddef>
#include <vector>
#include <utility>
#include "../core/domain.hpp"
#include "../core/relation_concepts.hpp"
#include "../storage/bit_matrix.hpp"

namespace discretex {

template <FiniteDomain Dom = index_domain>
class dense_relation {
public:
    using domain_type = Dom;

    explicit dense_relation(Dom domain)
        : domain_(std::move(domain)), matrix_(domain_.size(), domain_.size()) {}

    dense_relation(Dom domain, std::initializer_list<std::pair<std::size_t, std::size_t>> pairs)
        : dense_relation(std::move(domain)) {
        for (const auto& [u, v] : pairs) {
            add_pair(u, v);
        }
    }

    template <std::ranges::input_range PairRange>
    dense_relation(Dom domain, PairRange&& pairs)
        : dense_relation(std::move(domain)) {
        for (const auto& [u, v] : pairs) {
            add_pair(u, v);
        }
    }

    void add_pair(std::size_t u, std::size_t v) noexcept {
        matrix_.set(u, v, true);
    }

    void remove_pair(std::size_t u, std::size_t v) noexcept {
        matrix_.set(u, v, false);
    }

    [[nodiscard]] bool contains(std::size_t u, std::size_t v) const noexcept {
        return matrix_.test(u, v);
    }

    [[nodiscard]] std::size_t domain_size() const noexcept {
        return domain_.size();
    }

    [[nodiscard]] const Dom& domain() const noexcept {
        return domain_;
    }

    // Fiber iterator using countr_zero bit scanning
    [[nodiscard]] storage::bit_row_fiber_view out_neighbors(std::size_t u) const noexcept {
        return storage::bit_row_fiber_view(matrix_, u);
    }

    // In-place Warshall transitive closure using word-level bitwise OR
    void transitive_closure_inplace() noexcept {
        std::size_t n = domain_.size();
        for (std::size_t k = 0; k < n; ++k) {
            for (std::size_t i = 0; i < n; ++i) {
                if (matrix_.test(i, k)) {
                    matrix_.row_or(i, k);
                }
            }
        }
    }

private:
    Dom domain_;
    storage::bit_matrix matrix_;
};

// Free function for functional transitive closure
template <FiniteDomain Dom>
dense_relation<Dom> transitive_closure(dense_relation<Dom> rel) {
    rel.transitive_closure_inplace();
    return rel;
}

// Concept verification
static_assert(concepts::ForwardRelation<dense_relation<index_domain>>);

} // namespace discretex
