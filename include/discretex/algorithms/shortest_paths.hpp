#pragma once

#include <cstddef>
#include <vector>
#include <optional>
#include <queue>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include "../core/domain.hpp"
#include "../graph/weighted_graph.hpp"

namespace discretex::algorithms {

// Represents single-source shortest path results.
template <typename Weight = double>
struct shortest_path_result {
    std::size_t source{0};
    std::vector<std::optional<Weight>> distance;
    std::vector<std::optional<std::size_t>> predecessor;

    [[nodiscard]] bool is_reachable(std::size_t v) const noexcept {
        return v < distance.size() && distance[v].has_value();
    }
};

// Represents single-source shortest path results with negative cycle detection.
template <typename Weight = double>
struct bellman_ford_result {
    std::size_t source{0};
    bool has_negative_cycle{false};
    std::vector<std::optional<Weight>> distance;
    std::vector<std::optional<std::size_t>> predecessor;

    [[nodiscard]] bool is_reachable(std::size_t v) const noexcept {
        return v < distance.size() && distance[v].has_value();
    }
};

// Represents all-pairs shortest path results.
template <typename Weight = double>
struct all_pairs_shortest_path_result {
    bool has_negative_cycle{false};
    std::vector<std::vector<std::optional<Weight>>> distance;
    std::vector<std::vector<std::optional<std::size_t>>> next;

    [[nodiscard]] bool is_reachable(std::size_t u, std::size_t v) const noexcept {
        return u < distance.size() && v < distance[u].size() && distance[u][v].has_value();
    }
};

// Lazily reconstructs the path from source to target for single-source results.
template <typename Weight = double>
std::optional<std::vector<std::size_t>> reconstruct_path(
    const shortest_path_result<Weight>& res, std::size_t target) {
    if (!res.is_reachable(target)) return std::nullopt;

    std::vector<std::size_t> path;
    std::size_t curr = target;
    while (true) {
        path.push_back(curr);
        if (curr == res.source) break;
        auto pred = res.predecessor[curr];
        if (!pred.has_value()) {
            return std::nullopt;
        }
        curr = *pred;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

// Lazily reconstructs the path for Bellman-Ford results.
template <typename Weight = double>
std::optional<std::vector<std::size_t>> reconstruct_path(
    const bellman_ford_result<Weight>& res, std::size_t target) {
    if (res.has_negative_cycle || !res.is_reachable(target)) return std::nullopt;

    std::vector<std::size_t> path;
    std::size_t curr = target;
    while (true) {
        path.push_back(curr);
        if (curr == res.source) break;
        auto pred = res.predecessor[curr];
        if (!pred.has_value()) {
            return std::nullopt;
        }
        curr = *pred;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

// Lazily reconstructs the path between source and target for all-pairs results.
template <typename Weight = double>
std::optional<std::vector<std::size_t>> reconstruct_path(
    const all_pairs_shortest_path_result<Weight>& res,
    std::size_t source,
    std::size_t target) {
    if (!res.is_reachable(source, target) || res.has_negative_cycle) {
        return std::nullopt;
    }

    std::vector<std::size_t> path;
    std::size_t curr = source;
    path.push_back(curr);

    std::size_t n = res.distance.size();
    while (curr != target) {
        auto nxt = res.next[curr][target];
        if (!nxt.has_value()) return std::nullopt;
        curr = *nxt;
        path.push_back(curr);
        if (path.size() > n) {
            return std::nullopt; // cycle detected
        }
    }
    return path;
}

// DAG Shortest Paths via Topological Sorting.
// Complexity: O(V + E). Supports negative weights as long as graph is acyclic.
template <typename Weight = double, typename Graph>
shortest_path_result<Weight> dag_shortest_paths(const Graph& g, std::size_t source) {
    std::size_t n = g.vertex_count();
    if (source >= n) {
        throw std::out_of_range("Source vertex out of bounds in dag_shortest_paths");
    }

    // 1. Compute in-degrees and extract topological ordering
    std::vector<std::size_t> in_degree(n, 0);
    for (std::size_t u = 0; u < n; ++u) {
        for (const auto& nbr : g.neighbors(u)) {
            if (nbr.target < n) {
                ++in_degree[nbr.target];
            }
        }
    }

    std::queue<std::size_t> q;
    for (std::size_t i = 0; i < n; ++i) {
        if (in_degree[i] == 0) {
            q.push(i);
        }
    }

    std::vector<std::size_t> topo_order;
    topo_order.reserve(n);
    while (!q.empty()) {
        std::size_t u = q.front();
        q.pop();
        topo_order.push_back(u);

        for (const auto& nbr : g.neighbors(u)) {
            if (nbr.target < n) {
                if (--in_degree[nbr.target] == 0) {
                    q.push(nbr.target);
                }
            }
        }
    }

    if (topo_order.size() != n) {
        throw std::invalid_argument("Graph contains a cycle; dag_shortest_paths requires a directed acyclic graph");
    }

    // 2. Relax edges along topological order
    shortest_path_result<Weight> res;
    res.source = source;
    res.distance.assign(n, std::nullopt);
    res.predecessor.assign(n, std::nullopt);
    res.distance[source] = Weight{0};

    for (std::size_t u : topo_order) {
        if (res.distance[u].has_value()) {
            Weight dist_u = *res.distance[u];
            for (const auto& nbr : g.neighbors(u)) {
                std::size_t v = nbr.target;
                Weight cand = dist_u + nbr.weight;
                if (!res.distance[v].has_value() || cand < *res.distance[v]) {
                    res.distance[v] = cand;
                    res.predecessor[v] = u;
                }
            }
        }
    }

    return res;
}

// Dijkstra Shortest Paths using Min-Priority Queue.
// Complexity: O((V + E) log V). Requires non-negative edge weights.
template <typename Weight = double, typename Graph>
shortest_path_result<Weight> dijkstra_shortest_paths(const Graph& g, std::size_t source) {
    std::size_t n = g.vertex_count();
    if (source >= n) {
        throw std::out_of_range("Source vertex out of bounds in dijkstra_shortest_paths");
    }

    shortest_path_result<Weight> res;
    res.source = source;
    res.distance.assign(n, std::nullopt);
    res.predecessor.assign(n, std::nullopt);
    res.distance[source] = Weight{0};

    using pq_entry = std::pair<Weight, std::size_t>;
    std::priority_queue<pq_entry, std::vector<pq_entry>, std::greater<pq_entry>> pq;
    pq.push({Weight{0}, source});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > *res.distance[u]) continue;

        for (const auto& nbr : g.neighbors(u)) {
            if (nbr.weight < Weight{0}) {
                throw std::invalid_argument("dijkstra_shortest_paths requires non-negative edge weights");
            }
            std::size_t v = nbr.target;
            Weight cand = d + nbr.weight;
            if (!res.distance[v].has_value() || cand < *res.distance[v]) {
                res.distance[v] = cand;
                res.predecessor[v] = u;
                pq.push({cand, v});
            }
        }
    }

    return res;
}

// Bellman-Ford Shortest Paths with Negative Cycle Detection.
// Complexity: O(V * E). Relaxes all outgoing neighbors per vertex |V| - 1 times.
template <typename Weight = double, typename Graph>
bellman_ford_result<Weight> bellman_ford_shortest_paths(const Graph& g, std::size_t source) {
    std::size_t n = g.vertex_count();
    if (source >= n) {
        throw std::out_of_range("Source vertex out of bounds in bellman_ford_shortest_paths");
    }

    bellman_ford_result<Weight> res;
    res.source = source;
    res.has_negative_cycle = false;
    res.distance.assign(n, std::nullopt);
    res.predecessor.assign(n, std::nullopt);
    res.distance[source] = Weight{0};

    // Relax edges |V| - 1 times
    for (std::size_t iter = 0; iter + 1 < n; ++iter) {
        bool any_relaxed = false;
        for (std::size_t u = 0; u < n; ++u) {
            if (res.distance[u].has_value()) {
                Weight dist_u = *res.distance[u];
                for (const auto& nbr : g.neighbors(u)) {
                    std::size_t v = nbr.target;
                    Weight cand = dist_u + nbr.weight;
                    if (!res.distance[v].has_value() || cand < *res.distance[v]) {
                        res.distance[v] = cand;
                        res.predecessor[v] = u;
                        any_relaxed = true;
                    }
                }
            }
        }
        if (!any_relaxed) break;
    }

    // Pass |V|: check for reachable negative cycle
    for (std::size_t u = 0; u < n; ++u) {
        if (res.distance[u].has_value()) {
            Weight dist_u = *res.distance[u];
            for (const auto& nbr : g.neighbors(u)) {
                std::size_t v = nbr.target;
                Weight cand = dist_u + nbr.weight;
                if (!res.distance[v].has_value() || cand < *res.distance[v]) {
                    res.has_negative_cycle = true;
                    break;
                }
            }
        }
        if (res.has_negative_cycle) break;
    }

    return res;
}

// Floyd-Warshall All-Pairs Shortest Paths.
// Complexity: O(V^3).
template <typename Weight = double, typename Graph>
all_pairs_shortest_path_result<Weight> floyd_warshall_all_pairs(const Graph& g) {
    std::size_t n = g.vertex_count();
    all_pairs_shortest_path_result<Weight> res;
    res.has_negative_cycle = false;
    res.distance.assign(n, std::vector<std::optional<Weight>>(n, std::nullopt));
    res.next.assign(n, std::vector<std::optional<std::size_t>>(n, std::nullopt));

    for (std::size_t i = 0; i < n; ++i) {
        res.distance[i][i] = Weight{0};
        res.next[i][i] = i;
    }

    for (std::size_t u = 0; u < n; ++u) {
        for (const auto& nbr : g.neighbors(u)) {
            std::size_t v = nbr.target;
            if (v < n) {
                if (!res.distance[u][v].has_value() || nbr.weight < *res.distance[u][v]) {
                    res.distance[u][v] = nbr.weight;
                    res.next[u][v] = v;
                }
            }
        }
    }

    for (std::size_t k = 0; k < n; ++k) {
        for (std::size_t i = 0; i < n; ++i) {
            if (!res.distance[i][k].has_value()) continue;
            Weight d_ik = *res.distance[i][k];
            for (std::size_t j = 0; j < n; ++j) {
                if (!res.distance[k][j].has_value()) continue;
                Weight cand = d_ik + *res.distance[k][j];
                if (!res.distance[i][j].has_value() || cand < *res.distance[i][j]) {
                    res.distance[i][j] = cand;
                    res.next[i][j] = res.next[i][k];
                }
            }
        }
    }

    // Check diagonal for negative cycle
    for (std::size_t i = 0; i < n; ++i) {
        if (res.distance[i][i].has_value() && *res.distance[i][i] < Weight{0}) {
            res.has_negative_cycle = true;
            break;
        }
    }

    return res;
}

// Verifies that a path is structurally valid:
// 1. Path is non-empty and starts at source and ends at target.
// 2. Every consecutive pair of vertices forms an edge in g.
// 3. The sum of weights along the path strictly matches expected_distance.
template <typename Weight = double, typename Graph>
bool verify_path_integrity(const Graph& g,
                          const std::vector<std::size_t>& path,
                          Weight expected_distance) {
    if (path.empty()) return false;
    if (path.size() == 1) {
        return expected_distance == Weight{0};
    }

    Weight total_w{0};
    for (std::size_t i = 0; i + 1 < path.size(); ++i) {
        std::size_t u = path[i];
        std::size_t v = path[i + 1];
        auto w = g.edge_weight(u, v);
        if (!w.has_value()) return false;
        total_w += *w;
    }

    return total_w == expected_distance;
}

} // namespace discretex::algorithms
