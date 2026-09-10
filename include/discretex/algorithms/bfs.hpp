#pragma once
#include <vector>
#include <queue>
#include <cstddef>
#include "../core/relation_concepts.hpp"

namespace discretex {

template <concepts::ForwardRelation Rel>
std::vector<std::size_t> bfs_order(const Rel& rel, std::size_t start_vertex) {
    std::size_t n = rel.domain_size();
    if (start_vertex >= n) return {};

    std::vector<bool> visited(n, false);
    std::vector<std::size_t> order;
    order.reserve(n);

    std::queue<std::size_t> q;
    q.push(start_vertex);
    visited[start_vertex] = true;

    while (!q.empty()) {
        std::size_t u = q.front();
        q.pop();
        order.push_back(u);

        for (std::size_t v : rel.out_neighbors(u)) {
            if (!visited[v]) {
                visited[v] = true;
                q.push(v);
            }
        }
    }

    return order;
}

} // namespace discretex
