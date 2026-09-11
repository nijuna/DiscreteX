#pragma once

#include <cstddef>
#include <vector>
#include <utility>
#include <algorithm>
#include "../core/domain.hpp"
#include "../core/relation_concepts.hpp"

namespace discretex::algorithms {

/**
 * @brief Structured result containing cut vertices (articulation points) and bridge edges.
 */
struct biconnectivity_result {
    std::vector<std::size_t> cut_vertices;
    std::vector<std::pair<std::size_t, std::size_t>> bridges;
};

/**
 * @brief Find all articulation points (cut vertices) and bridges in an undirected graph using Tarjan low-link algorithm.
 *
 * Runs in linear time \(O(|V| + |E|)\).
 */
template <typename Graph>
inline biconnectivity_result find_cut_vertices_and_bridges(const Graph& g) {
    std::size_t n = 0;
    if constexpr (requires { g.vertex_count(); }) {
        n = g.vertex_count();
    } else if constexpr (requires { g.domain_size(); }) {
        n = g.domain_size();
    }

    std::vector<bool> visited(n, false);
    std::vector<std::size_t> tin(n, 0);
    std::vector<std::size_t> low(n, 0);
    std::size_t timer = 0;

    std::vector<bool> is_cut(n, false);
    std::vector<std::pair<std::size_t, std::size_t>> bridges;

    auto dfs = [&](auto& self, std::size_t u, std::size_t p) -> void {
        visited[u] = true;
        tin[u] = low[u] = ++timer;
        std::size_t children = 0;

        auto process_neighbor = [&](std::size_t v) {
            if (v == p) return;

            if (visited[v]) {
                low[u] = std::min(low[u], tin[v]);
            } else {
                self(self, v, u);
                low[u] = std::min(low[u], low[v]);
                if (low[v] >= tin[u] && p != static_cast<std::size_t>(-1)) {
                    is_cut[u] = true;
                }
                if (low[v] > tin[u]) {
                    bridges.emplace_back(std::min(u, v), std::max(u, v));
                }
                ++children;
            }
        };

        if constexpr (requires { g.neighbors(u); }) {
            for (const auto& nbr : g.neighbors(u)) {
                process_neighbor(nbr.target);
            }
        } else if constexpr (requires { g.out_neighbors(u); }) {
            for (std::size_t v : g.out_neighbors(u)) {
                process_neighbor(v);
            }
        }

        if (p == static_cast<std::size_t>(-1) && children > 1) {
            is_cut[u] = true;
        }
    };

    for (std::size_t i = 0; i < n; ++i) {
        if (!visited[i]) {
            dfs(dfs, i, static_cast<std::size_t>(-1));
        }
    }

    std::vector<std::size_t> cut_vertices;
    for (std::size_t i = 0; i < n; ++i) {
        if (is_cut[i]) {
            cut_vertices.push_back(i);
        }
    }

    std::sort(bridges.begin(), bridges.end());
    bridges.erase(std::unique(bridges.begin(), bridges.end()), bridges.end());

    return biconnectivity_result{std::move(cut_vertices), std::move(bridges)};
}

template <typename Graph>
inline std::vector<std::size_t> find_cut_vertices(const Graph& g) {
    return find_cut_vertices_and_bridges(g).cut_vertices;
}

template <typename Graph>
inline std::vector<std::pair<std::size_t, std::size_t>> find_bridges(const Graph& g) {
    return find_cut_vertices_and_bridges(g).bridges;
}

} // namespace discretex::algorithms
