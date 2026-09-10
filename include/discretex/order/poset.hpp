#pragma once
#include <cstddef>
#include <vector>
#include <optional>
#include <stdexcept>
#include "../core/domain.hpp"
#include "../core/relation_concepts.hpp"
#include "../relation/dense_relation.hpp"
#include "../graph/sparse_graph.hpp"
#include "../algorithms/topological_sort.hpp"

namespace discretex {

template <FiniteDomain Dom = index_domain>
class poset {
public:
    using domain_type = Dom;

    explicit poset(dense_relation<Dom> closure_rel)
        : closure_(std::move(closure_rel)) {
        validate_poset_axioms();
    }

    // Factory: Ingests an arbitrary DAG and computes its reflexive-transitive closure
    template <concepts::ForwardGraph G>
    static poset from_dag_closure(const G& g) {
        auto topo = topological_sort(g);
        if (!topo.has_value()) {
            throw std::invalid_argument("Input graph has directed cycles; cannot form a poset.");
        }

        std::size_t n = g.domain_size();
        dense_relation<Dom> rel(g.domain());

        // 1. Ingest edges and add reflexive self-loops
        for (std::size_t u = 0; u < n; ++u) {
            rel.add_pair(u, u); // Reflexivity
            for (std::size_t v : g.out_neighbors(u)) {
                rel.add_pair(u, v);
            }
        }

        // 2. Compute transitive closure
        rel.transitive_closure_inplace();

        return poset(std::move(rel));
    }

    // Factory: Ingests a pre-reduced Hasse diagram (covering relation DAG)
    template <concepts::ForwardGraph G>
    static poset from_hasse_diagram(const G& g) {
        std::size_t n = g.domain_size();

        // Must be irreflexive
        for (std::size_t u = 0; u < n; ++u) {
            if (g.contains(u, u)) {
                throw std::invalid_argument("Hasse diagram must be strictly irreflexive (no self-loops).");
            }
        }

        auto p = from_dag_closure(g);

        // Verify it was already transitively reduced (no shortcut edges)
        auto hasse = p.hasse_diagram();
        if (hasse.edge_count() != g.edge_count()) {
            throw std::invalid_argument("Input graph is a DAG but not transitively reduced (contains shortcut edges).");
        }

        return p;
    }

    [[nodiscard]] const Dom& domain() const noexcept {
        return closure_.domain();
    }

    [[nodiscard]] std::size_t domain_size() const noexcept {
        return closure_.domain_size();
    }

    // Reflexive partial order: u <= v
    [[nodiscard]] bool less_equal(std::size_t u, std::size_t v) const noexcept {
        return closure_.contains(u, v);
    }

    // Strict partial order: u < v
    [[nodiscard]] bool less(std::size_t u, std::size_t v) const noexcept {
        return (u != v) && closure_.contains(u, v);
    }

    [[nodiscard]] bool is_comparable(std::size_t u, std::size_t v) const noexcept {
        return less_equal(u, v) || less_equal(v, u);
    }

    // Computes the covering relation DAG (transitive reduction of strict order)
    [[nodiscard]] bidirectional_adjacency_graph<Dom> hasse_diagram() const {
        std::size_t n = domain_size();
        bidirectional_adjacency_graph<Dom> hasse(closure_.domain());

        // Strict relation matrix S where S[u][v] = (u < v)
        dense_relation<index_domain> strict_rel{index_domain(n)};
        for (std::size_t u = 0; u < n; ++u) {
            for (std::size_t v = 0; v < n; ++v) {
                if (less(u, v)) {
                    strict_rel.add_pair(u, v);
                }
            }
        }

        // Compute composition S o S (paths of length >= 2)
        dense_relation<index_domain> intermediate{index_domain(n)};
        for (std::size_t u = 0; u < n; ++u) {
            for (std::size_t w : strict_rel.out_neighbors(u)) {
                for (std::size_t v : strict_rel.out_neighbors(w)) {
                    intermediate.add_pair(u, v);
                }
            }
        }

        // Covering relation: (u < v) and NOT intermediate(u, v)
        for (std::size_t u = 0; u < n; ++u) {
            for (std::size_t v : strict_rel.out_neighbors(u)) {
                if (!intermediate.contains(u, v)) {
                    hasse.add_edge(u, v);
                }
            }
        }

        return hasse;
    }

    // Produces a valid linear extension via topological sort of the Hasse diagram
    [[nodiscard]] std::vector<std::size_t> linear_extension() const {
        auto hasse = hasse_diagram();
        auto order = topological_sort(hasse);
        if (!order.has_value()) {
            throw std::logic_error("Poset invariant broken: Hasse diagram cycle detected.");
        }
        return std::move(*order);
    }

    // Least upper bound (join / supremum: u v v)
    [[nodiscard]] std::optional<std::size_t> join(std::size_t u, std::size_t v) const {
        std::size_t n = domain_size();
        std::vector<std::size_t> upper_bounds;

        for (std::size_t w = 0; w < n; ++w) {
            if (less_equal(u, w) && less_equal(v, w)) {
                upper_bounds.push_back(w);
            }
        }

        for (std::size_t candidate : upper_bounds) {
            bool is_least = true;
            for (std::size_t other : upper_bounds) {
                if (!less_equal(candidate, other)) {
                    is_least = false;
                    break;
                }
            }
            if (is_least) return candidate;
        }

        return std::nullopt;
    }

    // Greatest lower bound (meet / infimum: u ^ v)
    [[nodiscard]] std::optional<std::size_t> meet(std::size_t u, std::size_t v) const {
        std::size_t n = domain_size();
        std::vector<std::size_t> lower_bounds;

        for (std::size_t w = 0; w < n; ++w) {
            if (less_equal(w, u) && less_equal(w, v)) {
                lower_bounds.push_back(w);
            }
        }

        for (std::size_t candidate : lower_bounds) {
            bool is_greatest = true;
            for (std::size_t other : lower_bounds) {
                if (!less_equal(other, candidate)) {
                    is_greatest = false;
                    break;
                }
            }
            if (is_greatest) return candidate;
        }

        return std::nullopt;
    }

    // Checks whether (P, <=) forms a lattice (unique join and meet for all pairs)
    [[nodiscard]] bool is_lattice() const {
        std::size_t n = domain_size();
        for (std::size_t u = 0; u < n; ++u) {
            for (std::size_t v = u; v < n; ++v) {
                if (!join(u, v).has_value() || !meet(u, v).has_value()) {
                    return false;
                }
            }
        }
        return true;
    }

private:
    void validate_poset_axioms() const {
        std::size_t n = closure_.domain_size();

        // 1. Reflexivity: u <= u
        for (std::size_t u = 0; u < n; ++u) {
            if (!closure_.contains(u, u)) {
                throw std::invalid_argument("Poset violation: Relation must be reflexive.");
            }
        }

        // 2. Antisymmetry: u <= v and v <= u ==> u == v
        for (std::size_t u = 0; u < n; ++u) {
            for (std::size_t v = 0; v < n; ++v) {
                if (u != v && closure_.contains(u, v) && closure_.contains(v, u)) {
                    throw std::invalid_argument("Poset violation: Relation must be antisymmetric (cycle detected).");
                }
            }
        }
    }

    dense_relation<Dom> closure_;
};

} // namespace discretex
