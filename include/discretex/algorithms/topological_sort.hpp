#pragma once
#include <vector>
#include <queue>
#include <cstddef>
#include <optional>
#include "../core/relation_concepts.hpp"

namespace discretex {

// Kahn's algorithm for linear extension / topological ordering
template <concepts::ForwardRelation Rel>
std::optional<std::vector<std::size_t>> topological_sort(const Rel& rel) {
    std::size_t n = rel.domain_size();
    std::vector<std::size_t> in_deg(n, 0);

    // Compute in-degrees
    for (std::size_t u = 0; u < n; ++u) {
        for (std::size_t v : rel.out_neighbors(u)) {
            if (v < n) {
                ++in_deg[v];
            }
        }
    }

    std::queue<std::size_t> q;
    for (std::size_t i = 0; i < n; ++i) {
        if (in_deg[i] == 0) {
            q.push(i);
        }
    }

    std::vector<std::size_t> order;
    order.reserve(n);

    while (!q.empty()) {
        std::size_t u = q.front();
        q.pop();
        order.push_back(u);

        for (std::size_t v : rel.out_neighbors(u)) {
            if (v < n) {
                --in_deg[v];
                if (in_deg[v] == 0) {
                    q.push(v);
                }
            }
        }
    }

    if (order.size() != n) {
        return std::nullopt; // Cycle detected: cannot linearly order
    }

    return order;
}

} // namespace discretex
