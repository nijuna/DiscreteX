#pragma once

#include <cstddef>
#include <vector>
#include <span>
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace discretex {

// Dedicated bipartite graph with explicit left/right partitions and side-local indexing:
// Left vertices:  0, ..., left_size - 1
// Right vertices: 0, ..., right_size - 1
class bipartite_graph {
public:
    bipartite_graph(std::size_t left_size, std::size_t right_size)
        : left_size_(left_size), right_size_(right_size),
          adj_left_(left_size), adj_right_(right_size) {}

    std::size_t left_size() const noexcept { return left_size_; }
    std::size_t right_size() const noexcept { return right_size_; }
    std::size_t edge_count() const noexcept { return edge_count_; }

    void add_edge(std::size_t u, std::size_t v) {
        if (u >= left_size_ || v >= right_size_) {
            throw std::out_of_range("Bipartite vertex index out of bounds");
        }
        auto it_l = std::lower_bound(adj_left_[u].begin(), adj_left_[u].end(), v);
        if (it_l == adj_left_[u].end() || *it_l != v) {
            adj_left_[u].insert(it_l, v);
            auto it_r = std::lower_bound(adj_right_[v].begin(), adj_right_[v].end(), u);
            adj_right_[v].insert(it_r, u);
            ++edge_count_;
        }
    }

    bool contains(std::size_t u, std::size_t v) const noexcept {
        if (u >= left_size_ || v >= right_size_) return false;
        return std::binary_search(adj_left_[u].begin(), adj_left_[u].end(), v);
    }

    std::span<const std::size_t> right_neighbors(std::size_t u) const {
        if (u >= left_size_) {
            throw std::out_of_range("Left vertex index out of bounds");
        }
        return std::span<const std::size_t>(adj_left_[u].data(), adj_left_[u].size());
    }

    std::span<const std::size_t> left_neighbors(std::size_t v) const {
        if (v >= right_size_) {
            throw std::out_of_range("Right vertex index out of bounds");
        }
        return std::span<const std::size_t>(adj_right_[v].data(), adj_right_[v].size());
    }

    static bipartite_graph from_edges(std::size_t left_size, std::size_t right_size,
                                      const std::vector<std::pair<std::size_t, std::size_t>>& edges) {
        bipartite_graph g(left_size, right_size);
        for (const auto& [u, v] : edges) {
            g.add_edge(u, v);
        }
        return g;
    }

private:
    std::size_t left_size_{0};
    std::size_t right_size_{0};
    std::size_t edge_count_{0};
    std::vector<std::vector<std::size_t>> adj_left_;
    std::vector<std::vector<std::size_t>> adj_right_;
};

} // namespace discretex
