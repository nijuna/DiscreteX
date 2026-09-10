#pragma once
#include <vector>
#include <span>
#include <algorithm>
#include <cstddef>
#include <utility>
#include <ranges>
#include "../core/domain.hpp"
#include "../core/relation_concepts.hpp"

namespace discretex {

// 1. Forward-only sparse graph
template <FiniteDomain Dom = index_domain>
class forward_adjacency_graph {
public:
    using domain_type = Dom;

    explicit forward_adjacency_graph(Dom domain)
        : domain_(std::move(domain)), out_edges_(domain_.size()) {}

    template <std::ranges::input_range EdgeRange>
        requires std::convertible_to<std::ranges::range_value_t<EdgeRange>, 
                                     std::pair<std::size_t, std::size_t>>
    static forward_adjacency_graph from_edges(Dom domain, EdgeRange&& edges) {
        forward_adjacency_graph g(std::move(domain));
        std::size_t n = g.domain_size();

        for (const auto& [u, v] : edges) {
            if (u < n && v < n) {
                g.out_edges_[u].push_back(v);
            }
        }

        g.edge_count_ = 0;
        for (std::size_t i = 0; i < n; ++i) {
            auto& out_vec = g.out_edges_[i];
            std::sort(out_vec.begin(), out_vec.end());
            out_vec.erase(std::unique(out_vec.begin(), out_vec.end()), out_vec.end());
            g.edge_count_ += out_vec.size();
        }

        return g;
    }

    void add_edge(std::size_t u, std::size_t v) {
        if (u >= out_edges_.size() || v >= domain_size()) return;
        auto& neighbors = out_edges_[u];
        auto it = std::lower_bound(neighbors.begin(), neighbors.end(), v);
        if (it == neighbors.end() || *it != v) {
            neighbors.insert(it, v);
            ++edge_count_;
        }
    }

    [[nodiscard]] bool contains(std::size_t u, std::size_t v) const noexcept {
        if (u >= out_edges_.size() || v >= domain_size()) return false;
        const auto& neighbors = out_edges_[u];
        return std::binary_search(neighbors.begin(), neighbors.end(), v);
    }

    [[nodiscard]] std::span<const std::size_t> out_neighbors(std::size_t u) const noexcept {
        if (u >= out_edges_.size()) return {};
        return out_edges_[u];
    }

    [[nodiscard]] std::size_t domain_size() const noexcept { return domain_.size(); }
    [[nodiscard]] std::size_t edge_count() const noexcept { return edge_count_; }
    [[nodiscard]] const Dom& domain() const noexcept { return domain_; }

private:
    Dom domain_;
    std::vector<std::vector<std::size_t>> out_edges_;
    std::size_t edge_count_{0};
};

// 2. Bidirectional sparse graph
template <FiniteDomain Dom = index_domain>
class bidirectional_adjacency_graph {
public:
    using domain_type = Dom;

    explicit bidirectional_adjacency_graph(Dom domain)
        : domain_(std::move(domain)), 
          out_edges_(domain_.size()), 
          in_edges_(domain_.size()) {}

    template <std::ranges::input_range EdgeRange>
        requires std::convertible_to<std::ranges::range_value_t<EdgeRange>, 
                                     std::pair<std::size_t, std::size_t>>
    static bidirectional_adjacency_graph from_edges(Dom domain, EdgeRange&& edges) {
        bidirectional_adjacency_graph g(std::move(domain));
        std::size_t n = g.domain_size();

        for (const auto& [u, v] : edges) {
            if (u < n && v < n) {
                g.out_edges_[u].push_back(v);
                g.in_edges_[v].push_back(u);
            }
        }

        g.edge_count_ = 0;
        for (std::size_t i = 0; i < n; ++i) {
            auto& out_vec = g.out_edges_[i];
            std::sort(out_vec.begin(), out_vec.end());
            out_vec.erase(std::unique(out_vec.begin(), out_vec.end()), out_vec.end());
            g.edge_count_ += out_vec.size();

            auto& in_vec = g.in_edges_[i];
            std::sort(in_vec.begin(), in_vec.end());
            in_vec.erase(std::unique(in_vec.begin(), in_vec.end()), in_vec.end());
        }

        return g;
    }

    void add_edge(std::size_t u, std::size_t v) {
        if (u >= out_edges_.size() || v >= domain_size()) return;
        auto& out_n = out_edges_[u];
        auto it_out = std::lower_bound(out_n.begin(), out_n.end(), v);
        if (it_out == out_n.end() || *it_out != v) {
            out_n.insert(it_out, v);

            auto& in_n = in_edges_[v];
            auto it_in = std::lower_bound(in_n.begin(), in_n.end(), u);
            in_n.insert(it_in, u);

            ++edge_count_;
        }
    }

    [[nodiscard]] bool contains(std::size_t u, std::size_t v) const noexcept {
        if (u >= out_edges_.size() || v >= domain_size()) return false;
        const auto& neighbors = out_edges_[u];
        return std::binary_search(neighbors.begin(), neighbors.end(), v);
    }

    [[nodiscard]] std::span<const std::size_t> out_neighbors(std::size_t u) const noexcept {
        if (u >= out_edges_.size()) return {};
        return out_edges_[u];
    }

    [[nodiscard]] std::span<const std::size_t> in_neighbors(std::size_t v) const noexcept {
        if (v >= in_edges_.size()) return {};
        return in_edges_[v];
    }

    [[nodiscard]] std::size_t in_degree(std::size_t v) const noexcept {
        if (v >= in_edges_.size()) return 0;
        return in_edges_[v].size();
    }

    [[nodiscard]] std::size_t out_degree(std::size_t u) const noexcept {
        if (u >= out_edges_.size()) return 0;
        return out_edges_[u].size();
    }

    [[nodiscard]] std::size_t domain_size() const noexcept { return domain_.size(); }
    [[nodiscard]] std::size_t edge_count() const noexcept { return edge_count_; }
    [[nodiscard]] const Dom& domain() const noexcept { return domain_; }

private:
    Dom domain_;
    std::vector<std::vector<std::size_t>> out_edges_;
    std::vector<std::vector<std::size_t>> in_edges_;
    std::size_t edge_count_{0};
};

// Concept verification
static_assert(concepts::ForwardGraph<forward_adjacency_graph<index_domain>>);
static_assert(concepts::BidirectionalGraph<bidirectional_adjacency_graph<index_domain>>);

} // namespace discretex
