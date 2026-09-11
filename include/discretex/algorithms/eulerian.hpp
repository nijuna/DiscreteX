#pragma once

#include <cstddef>
#include <vector>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include "../core/domain.hpp"
#include "../core/relation_concepts.hpp"

namespace discretex::algorithms {

/**
 * @brief Structured result of Eulerian trail and circuit search.
 */
struct eulerian_result {
    bool has_trail{false};
    bool is_circuit{false};
    std::vector<std::size_t> vertices; // Sequence of vertices in the Eulerian trail/circuit
};

/**
 * @brief Determine whether an undirected graph contains an Eulerian circuit.
 */
template <typename Graph>
inline bool has_eulerian_circuit_undirected(const Graph& g) {
    std::size_t n = 0;
    if constexpr (requires { g.vertex_count(); }) {
        n = g.vertex_count();
    } else if constexpr (requires { g.domain_size(); }) {
        n = g.domain_size();
    }
    if (n == 0) return true;

    std::size_t total_edges = 0;
    std::size_t start_vertex = n;

    for (std::size_t u = 0; u < n; ++u) {
        std::size_t deg = 0;
        if constexpr (requires { g.degree(u); }) {
            deg = g.degree(u);
        } else if constexpr (requires { g.out_neighbors(u); }) {
            deg = g.out_neighbors(u).size();
        }

        if (deg % 2 != 0) return false;
        if (deg > 0) {
            total_edges += deg;
            if (start_vertex == n) start_vertex = u;
        }
    }

    if (total_edges == 0) return true;

    // Check connectivity of non-isolated vertices via BFS
    std::vector<bool> visited(n, false);
    std::vector<std::size_t> queue;
    visited[start_vertex] = true;
    queue.push_back(start_vertex);

    std::size_t head = 0;
    while (head < queue.size()) {
        std::size_t u = queue[head++];
        if constexpr (requires { g.neighbors(u); }) {
            for (const auto& nbr : g.neighbors(u)) {
                if (!visited[nbr.target]) {
                    visited[nbr.target] = true;
                    queue.push_back(nbr.target);
                }
            }
        } else if constexpr (requires { g.out_neighbors(u); }) {
            for (std::size_t v : g.out_neighbors(u)) {
                if (!visited[v]) {
                    visited[v] = true;
                    queue.push_back(v);
                }
            }
        }
    }

    for (std::size_t u = 0; u < n; ++u) {
        std::size_t deg = 0;
        if constexpr (requires { g.degree(u); }) {
            deg = g.degree(u);
        } else if constexpr (requires { g.out_neighbors(u); }) {
            deg = g.out_neighbors(u).size();
        }
        if (deg > 0 && !visited[u]) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Determine whether an undirected graph contains an Eulerian trail.
 */
template <typename Graph>
inline bool has_eulerian_trail_undirected(const Graph& g) {
    std::size_t n = 0;
    if constexpr (requires { g.vertex_count(); }) {
        n = g.vertex_count();
    } else if constexpr (requires { g.domain_size(); }) {
        n = g.domain_size();
    }
    if (n == 0) return true;

    std::size_t odd_count = 0;
    std::size_t total_edges = 0;
    std::size_t start_vertex = n;

    for (std::size_t u = 0; u < n; ++u) {
        std::size_t deg = 0;
        if constexpr (requires { g.degree(u); }) {
            deg = g.degree(u);
        } else if constexpr (requires { g.out_neighbors(u); }) {
            deg = g.out_neighbors(u).size();
        }

        if (deg % 2 != 0) {
            ++odd_count;
            start_vertex = u; // Prefer odd vertex as start
        } else if (deg > 0 && start_vertex == n) {
            start_vertex = u;
        }
        total_edges += deg;
    }

    if (total_edges == 0) return true;
    if (odd_count != 0 && odd_count != 2) return false;

    // Check connectivity of non-isolated vertices
    std::vector<bool> visited(n, false);
    std::vector<std::size_t> queue;
    visited[start_vertex] = true;
    queue.push_back(start_vertex);

    std::size_t head = 0;
    while (head < queue.size()) {
        std::size_t u = queue[head++];
        if constexpr (requires { g.neighbors(u); }) {
            for (const auto& nbr : g.neighbors(u)) {
                if (!visited[nbr.target]) {
                    visited[nbr.target] = true;
                    queue.push_back(nbr.target);
                }
            }
        } else if constexpr (requires { g.out_neighbors(u); }) {
            for (std::size_t v : g.out_neighbors(u)) {
                if (!visited[v]) {
                    visited[v] = true;
                    queue.push_back(v);
                }
            }
        }
    }

    for (std::size_t u = 0; u < n; ++u) {
        std::size_t deg = 0;
        if constexpr (requires { g.degree(u); }) {
            deg = g.degree(u);
        } else if constexpr (requires { g.out_neighbors(u); }) {
            deg = g.out_neighbors(u).size();
        }
        if (deg > 0 && !visited[u]) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Construct an Eulerian trail or circuit on an undirected graph using Hierholzer algorithm.
 */
template <typename Graph>
inline eulerian_result find_eulerian_trail_undirected(const Graph& g) {
    std::size_t n = 0;
    if constexpr (requires { g.vertex_count(); }) {
        n = g.vertex_count();
    } else if constexpr (requires { g.domain_size(); }) {
        n = g.domain_size();
    }

    if (n == 0) {
        return eulerian_result{true, true, {}};
    }

    struct edge_record {
        std::size_t target;
        std::size_t edge_id;
    };

    std::vector<std::vector<edge_record>> adj(n);
    std::size_t edge_count = 0;

    for (std::size_t u = 0; u < n; ++u) {
        if constexpr (requires { g.neighbors(u); }) {
            for (const auto& nbr : g.neighbors(u)) {
                if (u <= nbr.target) {
                    std::size_t eid = edge_count++;
                    adj[u].push_back({nbr.target, eid});
                    if (u != nbr.target) {
                        adj[nbr.target].push_back({u, eid});
                    }
                }
            }
        } else if constexpr (requires { g.out_neighbors(u); }) {
            for (std::size_t v : g.out_neighbors(u)) {
                if (u <= v) {
                    std::size_t eid = edge_count++;
                    adj[u].push_back({v, eid});
                    if (u != v) {
                        adj[v].push_back({u, eid});
                    }
                }
            }
        }
    }

    if (edge_count == 0) {
        return eulerian_result{true, true, {0}};
    }

    std::size_t odd_count = 0;
    std::size_t start_v = n;
    for (std::size_t u = 0; u < n; ++u) {
        if (adj[u].size() % 2 != 0) {
            ++odd_count;
            start_v = u;
        } else if (adj[u].size() > 0 && start_v == n) {
            start_v = u;
        }
    }

    if (odd_count != 0 && odd_count != 2) {
        return eulerian_result{false, false, {}};
    }

    bool is_circ = (odd_count == 0);
    std::vector<bool> edge_used(edge_count, false);
    std::vector<std::size_t> head_idx(n, 0);

    std::vector<std::size_t> stack;
    std::vector<std::size_t> trail;
    stack.push_back(start_v);

    while (!stack.empty()) {
        std::size_t u = stack.back();
        while (head_idx[u] < adj[u].size() && edge_used[adj[u][head_idx[u]].edge_id]) {
            ++head_idx[u];
        }

        if (head_idx[u] < adj[u].size()) {
            auto [v, eid] = adj[u][head_idx[u]];
            edge_used[eid] = true;
            ++head_idx[u];
            stack.push_back(v);
        } else {
            trail.push_back(u);
            stack.pop_back();
        }
    }

    if (trail.size() != edge_count + 1) {
        return eulerian_result{false, false, {}}; // Disconnected components with edges
    }

    std::reverse(trail.begin(), trail.end());
    return eulerian_result{true, is_circ, std::move(trail)};
}

/**
 * @brief Determine whether a directed graph contains an Eulerian circuit.
 */
template <typename Graph>
inline bool has_eulerian_circuit_directed(const Graph& g) {
    std::size_t n = 0;
    if constexpr (requires { g.vertex_count(); }) {
        n = g.vertex_count();
    } else if constexpr (requires { g.domain_size(); }) {
        n = g.domain_size();
    }
    if (n == 0) return true;

    std::vector<std::size_t> in_deg(n, 0);
    std::vector<std::size_t> out_deg(n, 0);
    std::size_t total_edges = 0;
    std::size_t start_v = n;

    for (std::size_t u = 0; u < n; ++u) {
        out_deg[u] = g.out_neighbors(u).size();
        total_edges += out_deg[u];
        if (out_deg[u] > 0 && start_v == n) start_v = u;
        for (std::size_t v : g.out_neighbors(u)) {
            if (v < n) in_deg[v]++;
        }
    }

    if (total_edges == 0) return true;

    for (std::size_t u = 0; u < n; ++u) {
        if (in_deg[u] != out_deg[u]) return false;
    }

    // Check reachability from start_v
    std::vector<bool> visited(n, false);
    std::vector<std::size_t> queue;
    visited[start_v] = true;
    queue.push_back(start_v);

    std::size_t head = 0;
    while (head < queue.size()) {
        std::size_t u = queue[head++];
        for (std::size_t v : g.out_neighbors(u)) {
            if (!visited[v]) {
                visited[v] = true;
                queue.push_back(v);
            }
        }
    }

    for (std::size_t u = 0; u < n; ++u) {
        if ((out_deg[u] > 0 || in_deg[u] > 0) && !visited[u]) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Construct an Eulerian trail or circuit on a directed graph using Hierholzer algorithm.
 */
template <typename Graph>
inline eulerian_result find_eulerian_trail_directed(const Graph& g) {
    std::size_t n = 0;
    if constexpr (requires { g.vertex_count(); }) {
        n = g.vertex_count();
    } else if constexpr (requires { g.domain_size(); }) {
        n = g.domain_size();
    }
    if (n == 0) return eulerian_result{true, true, {}};

    std::vector<std::size_t> in_deg(n, 0);
    std::vector<std::size_t> out_deg(n, 0);
    std::size_t total_edges = 0;

    for (std::size_t u = 0; u < n; ++u) {
        out_deg[u] = g.out_neighbors(u).size();
        total_edges += out_deg[u];
        for (std::size_t v : g.out_neighbors(u)) {
            if (v < n) in_deg[v]++;
        }
    }

    if (total_edges == 0) {
        return eulerian_result{true, true, {0}};
    }

    std::size_t start_v = n;
    std::size_t start_count = 0;
    std::size_t end_count = 0;

    for (std::size_t u = 0; u < n; ++u) {
        if (out_deg[u] > in_deg[u]) {
            if (out_deg[u] - in_deg[u] == 1) {
                ++start_count;
                start_v = u;
            } else {
                return eulerian_result{false, false, {}};
            }
        } else if (in_deg[u] > out_deg[u]) {
            if (in_deg[u] - out_deg[u] == 1) {
                ++end_count;
            } else {
                return eulerian_result{false, false, {}};
            }
        }
    }

    bool is_circ = (start_count == 0 && end_count == 0);
    if (!is_circ && (start_count != 1 || end_count != 1)) {
        return eulerian_result{false, false, {}};
    }

    if (is_circ) {
        for (std::size_t u = 0; u < n; ++u) {
            if (out_deg[u] > 0) {
                start_v = u;
                break;
            }
        }
    }

    std::vector<std::size_t> head_idx(n, 0);
    std::vector<std::size_t> stack;
    std::vector<std::size_t> trail;
    stack.push_back(start_v);

    while (!stack.empty()) {
        std::size_t u = stack.back();
        auto neighbors = g.out_neighbors(u);
        if (head_idx[u] < neighbors.size()) {
            std::size_t v = neighbors[head_idx[u]++];
            stack.push_back(v);
        } else {
            trail.push_back(u);
            stack.pop_back();
        }
    }

    if (trail.size() != total_edges + 1) {
        return eulerian_result{false, false, {}};
    }

    std::reverse(trail.begin(), trail.end());
    return eulerian_result{true, is_circ, std::move(trail)};
}

} // namespace discretex::algorithms
