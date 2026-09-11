#pragma once

#include <cstddef>
#include <vector>
#include <span>
#include <optional>
#include <stdexcept>
#include <algorithm>
#include <concepts>
#include "../core/domain.hpp"

namespace discretex {

// Represents a weighted undirected edge connecting vertices u and v with weight.
template <typename Weight = double>
struct weighted_edge {
    std::size_t u{0};
    std::size_t v{0};
    Weight weight{};

    auto operator<=>(const weighted_edge&) const = default;
};

// Represents an incident neighbor and edge weight in an adjacency list.
template <typename Weight = double>
struct weighted_neighbor {
    std::size_t target{0};
    Weight weight{};

    auto operator<=>(const weighted_neighbor&) const = default;
};

// Represents an undirected graph with weighted edges over a finite domain.
template <typename Weight = double, FiniteDomain Dom = index_domain>
class weighted_undirected_graph {
public:
    using weight_type = Weight;
    using domain_type = Dom;
    using edge_type = weighted_edge<Weight>;
    using neighbor_type = weighted_neighbor<Weight>;

    explicit weighted_undirected_graph(Dom domain)
        : domain_(std::move(domain)), adj_(domain_.size()) {}

    explicit weighted_undirected_graph(std::size_t n = 0)
        requires std::same_as<Dom, index_domain>
        : domain_(index_domain(n)), adj_(n) {}

    // Adds an undirected edge between vertices u and v with given weight.
    void add_edge(std::size_t u, std::size_t v, Weight weight) {
        if (u >= domain_.size() || v >= domain_.size()) {
            throw std::out_of_range("Vertex index out of domain bounds in weighted_undirected_graph::add_edge");
        }
        edges_.push_back(weighted_edge<Weight>{u, v, weight});
        adj_[u].push_back(weighted_neighbor<Weight>{v, weight});
        if (u != v) {
            adj_[v].push_back(weighted_neighbor<Weight>{u, weight});
        }
    }

    [[nodiscard]] std::size_t domain_size() const noexcept { return domain_.size(); }
    [[nodiscard]] std::size_t vertex_count() const noexcept { return domain_.size(); }
    [[nodiscard]] std::size_t edge_count() const noexcept { return edges_.size(); }
    [[nodiscard]] const Dom& domain() const noexcept { return domain_; }

    [[nodiscard]] const std::vector<weighted_edge<Weight>>& edges() const noexcept {
        return edges_;
    }

    [[nodiscard]] std::span<const weighted_neighbor<Weight>> neighbors(std::size_t u) const {
        if (u >= adj_.size()) return {};
        return adj_[u];
    }

    [[nodiscard]] std::size_t degree(std::size_t u) const noexcept {
        if (u >= adj_.size()) return 0;
        return adj_[u].size();
    }

    // Checks whether an edge exists between u and v.
    [[nodiscard]] bool has_edge(std::size_t u, std::size_t v) const noexcept {
        if (u >= adj_.size() || v >= adj_.size()) return false;
        for (const auto& nbr : adj_[u]) {
            if (nbr.target == v) return true;
        }
        return false;
    }

    // Queries the edge weight between u and v (returning minimum weight if multiple edges exist).
    [[nodiscard]] std::optional<Weight> edge_weight(std::size_t u, std::size_t v) const noexcept {
        if (u >= adj_.size() || v >= adj_.size()) return std::nullopt;
        std::optional<Weight> min_w;
        for (const auto& nbr : adj_[u]) {
            if (nbr.target == v) {
                if (!min_w.has_value() || nbr.weight < *min_w) {
                    min_w = nbr.weight;
                }
            }
        }
        return min_w;
    }

private:
    Dom domain_;
    std::vector<weighted_edge<Weight>> edges_;
    std::vector<std::vector<weighted_neighbor<Weight>>> adj_;
};

// Represents a directed graph with weighted edges over a finite domain.
template <typename Weight = double, FiniteDomain Dom = index_domain>
class weighted_directed_graph {
public:
    using weight_type = Weight;
    using domain_type = Dom;
    using edge_type = weighted_edge<Weight>;
    using neighbor_type = weighted_neighbor<Weight>;

    explicit weighted_directed_graph(Dom domain)
        : domain_(std::move(domain)), adj_(domain_.size()) {}

    explicit weighted_directed_graph(std::size_t n = 0)
        requires std::same_as<Dom, index_domain>
        : domain_(index_domain(n)), adj_(n) {}

    // Adds a directed edge from u to v with given weight.
    void add_edge(std::size_t u, std::size_t v, Weight weight) {
        if (u >= domain_.size() || v >= domain_.size()) {
            throw std::out_of_range("Vertex index out of domain bounds in weighted_directed_graph::add_edge");
        }
        edges_.push_back(weighted_edge<Weight>{u, v, weight});
        adj_[u].push_back(weighted_neighbor<Weight>{v, weight});
    }

    [[nodiscard]] std::size_t domain_size() const noexcept { return domain_.size(); }
    [[nodiscard]] std::size_t vertex_count() const noexcept { return domain_.size(); }
    [[nodiscard]] std::size_t edge_count() const noexcept { return edges_.size(); }
    [[nodiscard]] const Dom& domain() const noexcept { return domain_; }

    [[nodiscard]] const std::vector<weighted_edge<Weight>>& edges() const noexcept {
        return edges_;
    }

    [[nodiscard]] std::span<const weighted_neighbor<Weight>> out_neighbors(std::size_t u) const {
        if (u >= adj_.size()) return {};
        return adj_[u];
    }

    [[nodiscard]] std::span<const weighted_neighbor<Weight>> neighbors(std::size_t u) const {
        return out_neighbors(u);
    }

    [[nodiscard]] std::size_t out_degree(std::size_t u) const noexcept {
        if (u >= adj_.size()) return 0;
        return adj_[u].size();
    }

    [[nodiscard]] std::size_t degree(std::size_t u) const noexcept {
        return out_degree(u);
    }

    [[nodiscard]] bool has_edge(std::size_t u, std::size_t v) const noexcept {
        if (u >= adj_.size() || v >= adj_.size()) return false;
        for (const auto& nbr : adj_[u]) {
            if (nbr.target == v) return true;
        }
        return false;
    }

    [[nodiscard]] std::optional<Weight> edge_weight(std::size_t u, std::size_t v) const noexcept {
        if (u >= adj_.size() || v >= adj_.size()) return std::nullopt;
        std::optional<Weight> min_w;
        for (const auto& nbr : adj_[u]) {
            if (nbr.target == v) {
                if (!min_w.has_value() || nbr.weight < *min_w) {
                    min_w = nbr.weight;
                }
            }
        }
        return min_w;
    }

private:
    Dom domain_;
    std::vector<weighted_edge<Weight>> edges_;
    std::vector<std::vector<weighted_neighbor<Weight>>> adj_;
};

} // namespace discretex
