#pragma once

#include <cstddef>
#include <vector>
#include <span>
#include <stdexcept>
#include <concepts>
#include <utility>
#include "../core/domain.hpp"

namespace discretex {

// Represents an original directed edge with capacity in a flow network.
template <typename Capacity = long long>
struct flow_edge {
    std::size_t from{0};
    std::size_t to{0};
    Capacity capacity{0};

    auto operator<=>(const flow_edge&) const = default;
};

// Represents a directed edge in the residual graph.
// Maintains stable mutual indices to its paired reverse residual edge.
template <typename Capacity = long long>
struct residual_edge {
    std::size_t to{0};
    Capacity residual{0};
    std::size_t rev{0};
    std::size_t original_edge_index{0};
    bool is_forward{true};

    auto operator<=>(const residual_edge&) const = default;
};

// Dedicated flow network structure over a finite domain.
// Maintains original capacity edges alongside paired residual structures.
template <typename Capacity = long long, FiniteDomain Dom = index_domain>
class flow_network {
public:
    using capacity_type = Capacity;
    using domain_type = Dom;
    using edge_type = flow_edge<Capacity>;
    using residual_edge_type = residual_edge<Capacity>;

    explicit flow_network(Dom domain)
        : domain_(std::move(domain)), adj_(domain_.size()) {}

    explicit flow_network(std::size_t n = 0)
        requires std::same_as<Dom, index_domain>
        : domain_(index_domain(n)), adj_(n) {}

    // Adds a directed edge from u to v with specified capacity.
    // Creates mutually linked forward and backward residual edges.
    std::size_t add_edge(std::size_t u, std::size_t v, Capacity capacity) {
        if (u >= domain_.size() || v >= domain_.size()) {
            throw std::out_of_range("Vertex index out of bounds in flow_network::add_edge");
        }
        if (capacity < Capacity{0}) {
            throw std::invalid_argument("Capacity must be non-negative");
        }

        std::size_t edge_idx = edges_.size();
        edges_.push_back(flow_edge<Capacity>{u, v, capacity});

        std::size_t u_idx = adj_[u].size();
        std::size_t v_idx = adj_[v].size();

        if (u == v) {
            adj_[u].push_back(residual_edge<Capacity>{u, capacity, u_idx + 1, edge_idx, true});
            adj_[u].push_back(residual_edge<Capacity>{u, Capacity{0}, u_idx, edge_idx, false});
        } else {
            adj_[u].push_back(residual_edge<Capacity>{v, capacity, v_idx, edge_idx, true});
            adj_[v].push_back(residual_edge<Capacity>{u, Capacity{0}, u_idx, edge_idx, false});
        }

        return edge_idx;
    }

    [[nodiscard]] std::size_t domain_size() const noexcept { return domain_.size(); }
    [[nodiscard]] std::size_t vertex_count() const noexcept { return domain_.size(); }
    [[nodiscard]] std::size_t edge_count() const noexcept { return edges_.size(); }
    [[nodiscard]] const Dom& domain() const noexcept { return domain_; }

    [[nodiscard]] const std::vector<flow_edge<Capacity>>& original_edges() const noexcept {
        return edges_;
    }

    [[nodiscard]] const std::vector<std::vector<residual_edge<Capacity>>>& residual_adjacency() const noexcept {
        return adj_;
    }

    [[nodiscard]] std::span<const residual_edge<Capacity>> residual_edges(std::size_t u) const {
        if (u >= adj_.size()) return {};
        return adj_[u];
    }

private:
    Dom domain_;
    std::vector<flow_edge<Capacity>> edges_;
    std::vector<std::vector<residual_edge<Capacity>>> adj_;
};

} // namespace discretex
