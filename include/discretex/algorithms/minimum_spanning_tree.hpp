#pragma once

#include <cstddef>
#include <vector>
#include <queue>
#include <algorithm>
#include <tuple>
#include "../core/domain.hpp"
#include "../core/dsu.hpp"
#include "../graph/weighted_graph.hpp"

namespace discretex::algorithms {

// Represents the result of a minimum spanning tree / forest computation.
template <typename Weight = double>
struct spanning_forest_result {
    std::vector<weighted_edge<Weight>> edges;
    Weight total_weight{0};
    std::size_t component_count{0};
    bool is_connected{false};

    [[nodiscard]] std::size_t edge_count() const noexcept {
        return edges.size();
    }
};

// Computes the connected components of a weighted undirected graph using Disjoint Set Union.
template <typename Weight = double, FiniteDomain Dom = index_domain>
inline std::vector<std::vector<std::size_t>> connected_components(
    const weighted_undirected_graph<Weight, Dom>& g) {
    disjoint_set dsu(g.vertex_count());
    for (const auto& e : g.edges()) {
        dsu.unite(e.u, e.v);
    }
    return dsu.components();
}

// Computes the number of connected components in a weighted undirected graph.
template <typename Weight = double, FiniteDomain Dom = index_domain>
inline std::size_t connected_component_count(
    const weighted_undirected_graph<Weight, Dom>& g) {
    disjoint_set dsu(g.vertex_count());
    for (const auto& e : g.edges()) {
        dsu.unite(e.u, e.v);
    }
    return dsu.component_count();
}

// Kruskal Minimum Spanning Forest Algorithm.
// Complexity: O(E log E + E alpha(V)).
template <typename Weight = double, FiniteDomain Dom = index_domain>
spanning_forest_result<Weight> minimum_spanning_tree_kruskal(
    const weighted_undirected_graph<Weight, Dom>& g) {
    std::size_t n = g.vertex_count();
    spanning_forest_result<Weight> result;
    if (n == 0) {
        result.is_connected = true;
        return result;
    }

    auto edge_list = g.edges();
    std::stable_sort(edge_list.begin(), edge_list.end(), [](const auto& a, const auto& b) {
        return a.weight < b.weight;
    });

    disjoint_set dsu(n);
    result.edges.reserve(n > 0 ? n - 1 : 0);

    for (const auto& edge : edge_list) {
        if (dsu.unite(edge.u, edge.v)) {
            result.edges.push_back(edge);
            result.total_weight += edge.weight;
        }
    }

    result.component_count = dsu.component_count();
    result.is_connected = (result.component_count <= 1);
    return result;
}

// Prim Minimum Spanning Forest Algorithm using min-priority queue.
// Complexity: O(E log V).
template <typename Weight = double, FiniteDomain Dom = index_domain>
spanning_forest_result<Weight> minimum_spanning_tree_prim(
    const weighted_undirected_graph<Weight, Dom>& g) {
    std::size_t n = g.vertex_count();
    spanning_forest_result<Weight> result;
    if (n == 0) {
        result.is_connected = true;
        return result;
    }

    std::vector<bool> visited(n, false);

    struct prim_entry {
        Weight weight;
        std::size_t from;
        std::size_t to;

        bool operator>(const prim_entry& other) const noexcept {
            return weight > other.weight;
        }
    };

    std::priority_queue<prim_entry, std::vector<prim_entry>, std::greater<prim_entry>> pq;
    std::size_t components_found = 0;

    for (std::size_t s = 0; s < n; ++s) {
        if (visited[s]) continue;

        ++components_found;
        visited[s] = true;

        for (const auto& nbr : g.neighbors(s)) {
            pq.push({nbr.weight, s, nbr.target});
        }

        while (!pq.empty()) {
            auto [w, u, v] = pq.top();
            pq.pop();

            if (visited[v]) continue;

            visited[v] = true;
            result.edges.push_back(weighted_edge<Weight>{u, v, w});
            result.total_weight += w;

            for (const auto& nbr : g.neighbors(v)) {
                if (!visited[nbr.target]) {
                    pq.push({nbr.weight, v, nbr.target});
                }
            }
        }
    }

    result.component_count = components_found;
    result.is_connected = (components_found <= 1);
    return result;
}

// Canonical Minimum Spanning Tree dispatch.
template <typename Weight = double, FiniteDomain Dom = index_domain>
inline spanning_forest_result<Weight> minimum_spanning_tree(
    const weighted_undirected_graph<Weight, Dom>& g) {
    return minimum_spanning_tree_kruskal(g);
}

// Verifies structural invariants of a candidate minimum spanning forest:
// 1. Every edge in the forest exists in the underlying graph.
// 2. The edge set is strictly acyclic.
// 3. The edge count is exactly |V| - c, where c is the number of connected components in g.
// 4. Connectivity between vertices is identical to that of g.
template <typename Weight = double, FiniteDomain Dom = index_domain>
bool is_valid_spanning_forest(const weighted_undirected_graph<Weight, Dom>& g,
                             const spanning_forest_result<Weight>& forest) {
    std::size_t n = g.vertex_count();
    if (n == 0) {
        return forest.edges.empty() && forest.component_count == 0;
    }

    disjoint_set forest_dsu(n);
    for (const auto& e : forest.edges) {
        if (e.u >= n || e.v >= n) return false;
        if (!g.has_edge(e.u, e.v)) return false;
        // Uniting must succeed; if already connected, a cycle is present!
        if (!forest_dsu.unite(e.u, e.v)) {
            return false;
        }
    }

    disjoint_set graph_dsu(n);
    for (const auto& e : g.edges()) {
        graph_dsu.unite(e.u, e.v);
    }

    if (forest_dsu.component_count() != graph_dsu.component_count()) {
        return false;
    }

    if (forest.edges.size() != n - graph_dsu.component_count()) {
        return false;
    }

    return true;
}

} // namespace discretex::algorithms
