#pragma once

#include <cstddef>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include "../core/domain.hpp"
#include "../graph/flow_network.hpp"

namespace discretex::algorithms {

// Represents the result of a maximum flow computation.
template <typename Capacity = long long>
struct flow_result {
    Capacity max_flow{0};
    std::vector<bool> source_side_min_cut; // true for vertices on source side S
    std::vector<Capacity> edge_flows;      // flow through each original edge
};

// Computes the capacity of an (S, T) cut given a boolean characteristic vector for S.
template <typename Capacity = long long, FiniteDomain Dom = index_domain>
Capacity compute_cut_capacity(const flow_network<Capacity, Dom>& net,
                             const std::vector<bool>& source_side) {
    Capacity cap = 0;
    for (const auto& e : net.original_edges()) {
        if (source_side[e.from] && !source_side[e.to]) {
            cap += e.capacity;
        }
    }
    return cap;
}

// Verifies flow conservation and capacity constraints:
// 1. 0 <= flow(e) <= capacity(e)
// 2. Inflow equals outflow for all vertices v not in {source, sink}
// 3. Net outflow from source == max_flow == Net inflow to sink
template <typename Capacity = long long, FiniteDomain Dom = index_domain>
bool verify_flow_conservation(const flow_network<Capacity, Dom>& net,
                             const flow_result<Capacity>& res,
                             std::size_t source,
                             std::size_t sink) {
    std::size_t n = net.vertex_count();
    if (res.edge_flows.size() != net.edge_count()) return false;

    // 1. Capacity constraints
    const auto& edges = net.original_edges();
    for (std::size_t i = 0; i < edges.size(); ++i) {
        if (res.edge_flows[i] < 0 || res.edge_flows[i] > edges[i].capacity) {
            return false;
        }
    }

    // 2. Vertex net flow balances
    std::vector<Capacity> net_outflow(n, Capacity{0});
    for (std::size_t i = 0; i < edges.size(); ++i) {
        const auto& e = edges[i];
        net_outflow[e.from] += res.edge_flows[i];
        net_outflow[e.to] -= res.edge_flows[i];
    }

    if (source < n && sink < n && source != sink) {
        if (net_outflow[source] != res.max_flow) return false;
        if (net_outflow[sink] != -res.max_flow) return false;

        for (std::size_t u = 0; u < n; ++u) {
            if (u != source && u != sink) {
                if (net_outflow[u] != Capacity{0}) return false;
            }
        }
    }

    return true;
}

// Internal helper to reconstruct min-cut and per-edge flows from final residual state.
template <typename Capacity = long long, FiniteDomain Dom = index_domain>
flow_result<Capacity> build_flow_result(
    const flow_network<Capacity, Dom>& net,
    const std::vector<std::vector<residual_edge<Capacity>>>& res_adj,
    Capacity total_flow,
    std::size_t source) {
    std::size_t n = net.vertex_count();
    flow_result<Capacity> res;
    res.max_flow = total_flow;
    res.source_side_min_cut.assign(n, false);
    res.edge_flows.assign(net.edge_count(), Capacity{0});

    if (source < n) {
        std::queue<std::size_t> q;
        res.source_side_min_cut[source] = true;
        q.push(source);

        while (!q.empty()) {
            std::size_t u = q.front();
            q.pop();

            for (const auto& e : res_adj[u]) {
                if (e.residual > Capacity{0} && !res.source_side_min_cut[e.to]) {
                    res.source_side_min_cut[e.to] = true;
                    q.push(e.to);
                }
            }
        }
    }

    // Reconstruct flow on each original edge: flow = capacity - forward_residual
    const auto& orig_edges = net.original_edges();
    for (std::size_t u = 0; u < n; ++u) {
        for (const auto& e : res_adj[u]) {
            if (e.is_forward && e.original_edge_index < orig_edges.size()) {
                res.edge_flows[e.original_edge_index] =
                    orig_edges[e.original_edge_index].capacity - e.residual;
            }
        }
    }

    return res;
}

// Edmonds-Karp Max-Flow Algorithm using BFS augmenting paths.
// Complexity: O(V * E^2).
template <typename Capacity = long long, FiniteDomain Dom = index_domain>
flow_result<Capacity> max_flow_edmonds_karp(
    const flow_network<Capacity, Dom>& net,
    std::size_t source,
    std::size_t sink) {
    std::size_t n = net.vertex_count();
    if (source >= n || sink >= n || source == sink) {
        return build_flow_result(net, net.residual_adjacency(), Capacity{0}, source);
    }

    auto res_adj = net.residual_adjacency();
    Capacity total_flow = 0;

    std::vector<std::size_t> parent_vertex(n);
    std::vector<std::size_t> parent_edge_idx(n);

    while (true) {
        std::fill(parent_vertex.begin(), parent_vertex.end(), n);
        std::queue<std::size_t> q;
        q.push(source);
        parent_vertex[source] = source;

        while (!q.empty() && parent_vertex[sink] == n) {
            std::size_t u = q.front();
            q.pop();

            for (std::size_t idx = 0; idx < res_adj[u].size(); ++idx) {
                const auto& e = res_adj[u][idx];
                if (e.residual > Capacity{0} && parent_vertex[e.to] == n) {
                    parent_vertex[e.to] = u;
                    parent_edge_idx[e.to] = idx;
                    q.push(e.to);
                }
            }
        }

        if (parent_vertex[sink] == n) {
            break; // No augmenting path remains
        }

        // Compute bottleneck capacity
        Capacity bottleneck = std::numeric_limits<Capacity>::max();
        for (std::size_t curr = sink; curr != source; curr = parent_vertex[curr]) {
            std::size_t p = parent_vertex[curr];
            std::size_t idx = parent_edge_idx[curr];
            bottleneck = std::min(bottleneck, res_adj[p][idx].residual);
        }

        // Apply augmentation
        for (std::size_t curr = sink; curr != source; curr = parent_vertex[curr]) {
            std::size_t p = parent_vertex[curr];
            std::size_t idx = parent_edge_idx[curr];
            auto& fwd = res_adj[p][idx];
            auto& rev = res_adj[fwd.to][fwd.rev];
            fwd.residual -= bottleneck;
            rev.residual += bottleneck;
        }

        total_flow += bottleneck;
    }

    return build_flow_result(net, res_adj, total_flow, source);
}

// Dinic Max-Flow Algorithm using BFS level-graphs and DFS blocking flows.
// Complexity: O(V^2 * E), and O(E * sqrt(V)) on unit networks.
template <typename Capacity = long long, FiniteDomain Dom = index_domain>
flow_result<Capacity> max_flow_dinic(
    const flow_network<Capacity, Dom>& net,
    std::size_t source,
    std::size_t sink) {
    std::size_t n = net.vertex_count();
    if (source >= n || sink >= n || source == sink) {
        return build_flow_result(net, net.residual_adjacency(), Capacity{0}, source);
    }

    auto res_adj = net.residual_adjacency();
    Capacity total_flow = 0;

    std::vector<int> level(n);
    std::vector<std::size_t> ptr(n);

    auto bfs = [&]() -> bool {
        std::fill(level.begin(), level.end(), -1);
        level[source] = 0;
        std::queue<std::size_t> q;
        q.push(source);

        while (!q.empty()) {
            std::size_t u = q.front();
            q.pop();

            for (const auto& e : res_adj[u]) {
                if (e.residual > Capacity{0} && level[e.to] == -1) {
                    level[e.to] = level[u] + 1;
                    q.push(e.to);
                }
            }
        }
        return level[sink] != -1;
    };

    auto dfs = [&](auto& self, std::size_t u, Capacity pushed) -> Capacity {
        if (pushed == Capacity{0} || u == sink) return pushed;

        for (std::size_t& cid = ptr[u]; cid < res_adj[u].size(); ++cid) {
            auto& fwd = res_adj[u][cid];
            std::size_t trg = fwd.to;

            if (level[u] + 1 != level[trg] || fwd.residual == Capacity{0}) {
                continue;
            }

            Capacity tr = self(self, trg, std::min(pushed, fwd.residual));
            if (tr == Capacity{0}) continue;

            fwd.residual -= tr;
            res_adj[trg][fwd.rev].residual += tr;
            return tr;
        }
        return Capacity{0};
    };

    while (bfs()) {
        std::fill(ptr.begin(), ptr.end(), 0);
        while (Capacity pushed = dfs(dfs, source, std::numeric_limits<Capacity>::max())) {
            total_flow += pushed;
        }
    }

    return build_flow_result(net, res_adj, total_flow, source);
}

// Canonical Maximum Flow dispatch.
template <typename Capacity = long long, FiniteDomain Dom = index_domain>
inline flow_result<Capacity> max_flow(
    const flow_network<Capacity, Dom>& net,
    std::size_t source,
    std::size_t sink) {
    return max_flow_dinic(net, source, sink);
}

} // namespace discretex::algorithms
