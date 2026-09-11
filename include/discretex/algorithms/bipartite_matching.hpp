#pragma once

#include <cstddef>
#include <vector>
#include <queue>
#include <utility>
#include <algorithm>
#include "../graph/bipartite_graph.hpp"

namespace discretex::algorithms {

struct matching_result {
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);
    std::size_t matching_size{0};
    std::vector<std::size_t> mate_left;  // left vertex u -> right vertex v, or npos
    std::vector<std::size_t> mate_right; // right vertex v -> left vertex u, or npos

    bool is_matched_left(std::size_t u) const noexcept {
        return u < mate_left.size() && mate_left[u] != npos;
    }
    bool is_matched_right(std::size_t v) const noexcept {
        return v < mate_right.size() && mate_right[v] != npos;
    }

    std::vector<std::pair<std::size_t, std::size_t>> edges() const {
        std::vector<std::pair<std::size_t, std::size_t>> res;
        res.reserve(matching_size);
        for (std::size_t u = 0; u < mate_left.size(); ++u) {
            if (mate_left[u] != npos) {
                res.emplace_back(u, mate_left[u]);
            }
        }
        return res;
    }
};

// Maximum bipartite matching via Kuhn augmenting-path algorithm (O(V * E)).
inline matching_result maximum_bipartite_matching_kuhn(const bipartite_graph& g) {
    std::size_t n_l = g.left_size();
    std::size_t n_r = g.right_size();

    matching_result res;
    res.mate_left.assign(n_l, matching_result::npos);
    res.mate_right.assign(n_r, matching_result::npos);

    std::vector<bool> visited(n_r, false);

    auto dfs = [&](auto& self, std::size_t u) -> bool {
        for (std::size_t v : g.right_neighbors(u)) {
            if (visited[v]) continue;
            visited[v] = true;

            if (res.mate_right[v] == matching_result::npos || self(self, res.mate_right[v])) {
                res.mate_right[v] = u;
                res.mate_left[u] = v;
                return true;
            }
        }
        return false;
    };

    for (std::size_t u = 0; u < n_l; ++u) {
        std::fill(visited.begin(), visited.end(), false);
        if (dfs(dfs, u)) {
            ++res.matching_size;
        }
    }
    return res;
}

// Maximum bipartite matching via Hopcroft-Karp algorithm (O(E * sqrt(V))).
inline matching_result maximum_bipartite_matching_hopcroft_karp(const bipartite_graph& g) {
    std::size_t n_l = g.left_size();
    std::size_t n_r = g.right_size();

    matching_result res;
    res.mate_left.assign(n_l, matching_result::npos);
    res.mate_right.assign(n_r, matching_result::npos);

    static constexpr std::size_t INF = static_cast<std::size_t>(-1);
    std::vector<std::size_t> dist(n_l, INF);
    std::vector<std::size_t> q(n_l);

    auto bfs = [&]() -> bool {
        std::size_t q_head = 0, q_tail = 0;
        for (std::size_t u = 0; u < n_l; ++u) {
            if (res.mate_left[u] == matching_result::npos) {
                dist[u] = 0;
                q[q_tail++] = u;
            } else {
                dist[u] = INF;
            }
        }

        std::size_t dist_nil = INF;
        while (q_head < q_tail) {
            std::size_t u = q[q_head++];
            if (dist[u] < dist_nil) {
                for (std::size_t v : g.right_neighbors(u)) {
                    std::size_t next_u = res.mate_right[v];
                    if (next_u == matching_result::npos) {
                        dist_nil = dist[u] + 1;
                    } else if (dist[next_u] == INF) {
                        dist[next_u] = dist[u] + 1;
                        q[q_tail++] = next_u;
                    }
                }
            }
        }
        return dist_nil != INF;
    };

    auto dfs = [&](auto& self, std::size_t u) -> bool {
        for (std::size_t v : g.right_neighbors(u)) {
            std::size_t next_u = res.mate_right[v];
            if (next_u == matching_result::npos) {
                res.mate_left[u] = v;
                res.mate_right[v] = u;
                return true;
            } else if (dist[next_u] == dist[u] + 1) {
                if (self(self, next_u)) {
                    res.mate_left[u] = v;
                    res.mate_right[v] = u;
                    return true;
                }
            }
        }
        dist[u] = INF;
        return false;
    };

    while (bfs()) {
        for (std::size_t u = 0; u < n_l; ++u) {
            if (res.mate_left[u] == matching_result::npos && dfs(dfs, u)) {
                ++res.matching_size;
            }
        }
    }
    return res;
}

// Canonical maximum bipartite matching function (defaults to Hopcroft-Karp).
inline matching_result maximum_bipartite_matching(const bipartite_graph& g) {
    return maximum_bipartite_matching_hopcroft_karp(g);
}

struct vertex_cover_result {
    std::vector<std::size_t> left_cover;
    std::vector<std::size_t> right_cover;

    std::size_t size() const noexcept {
        return left_cover.size() + right_cover.size();
    }
};

// Koenig Theorem: In any bipartite graph, max matching size == min vertex cover size (|M| == |C|).
// Computes minimum vertex cover via alternating path reachability from unmatched left vertices.
inline vertex_cover_result minimum_vertex_cover(const bipartite_graph& g, const matching_result& m) {
    std::size_t n_l = g.left_size();
    std::size_t n_r = g.right_size();

    std::vector<bool> z_left(n_l, false);
    std::vector<bool> z_right(n_r, false);

    std::vector<std::size_t> q;
    for (std::size_t u = 0; u < n_l; ++u) {
        if (!m.is_matched_left(u)) {
            z_left[u] = true;
            q.push_back(u);
        }
    }

    std::size_t head = 0;
    while (head < q.size()) {
        std::size_t u = q[head++];
        for (std::size_t v : g.right_neighbors(u)) {
            if (m.mate_left[u] != v && !z_right[v]) {
                z_right[v] = true;
                std::size_t matched_l = m.mate_right[v];
                if (matched_l != matching_result::npos && !z_left[matched_l]) {
                    z_left[matched_l] = true;
                    q.push_back(matched_l);
                }
            }
        }
    }

    vertex_cover_result cover;
    for (std::size_t u = 0; u < n_l; ++u) {
        if (!z_left[u]) {
            cover.left_cover.push_back(u);
        }
    }
    for (std::size_t v = 0; v < n_r; ++v) {
        if (z_right[v]) {
            cover.right_cover.push_back(v);
        }
    }
    return cover;
}

} // namespace discretex::algorithms
