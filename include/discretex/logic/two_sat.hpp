#pragma once

#include <cstddef>
#include <vector>
#include <optional>
#include <algorithm>
#include <stdexcept>
#include <initializer_list>
#include "normal_forms.hpp"
#include "../algorithms/tarjan_scc.hpp"
#include "../graph/sparse_graph.hpp"
#include "../core/domain.hpp"

namespace discretex::logic {

// A 2-clause: (left or right)
struct clause2 {
    literal left;
    literal right;

    bool operator==(const clause2& o) const = default;
};

// Represents a 2-CNF formula (conjunction of 2-clauses).
class formula_2cnf {
public:
    formula_2cnf() = default;
    explicit formula_2cnf(std::size_t num_vars) : num_vars_(num_vars) {}
    formula_2cnf(std::size_t num_vars, std::initializer_list<clause2> clauses)
        : num_vars_(num_vars), clauses_(clauses) {}

    void add_clause(literal a, literal b) {
        clauses_.push_back({a, b});
        num_vars_ = std::max(num_vars_, std::max(a.var, b.var) + 1);
    }

    void add_clause(clause2 c) {
        add_clause(c.left, c.right);
    }

    std::size_t variable_count() const noexcept { return num_vars_; }
    const std::vector<clause2>& clauses() const noexcept { return clauses_; }

private:
    std::size_t num_vars_{0};
    std::vector<clause2> clauses_;
};

// Constructs the directed implication graph from a 2-CNF formula.
// Each clause (a or b) is represented as two directed implications:
// (~a -> b) and (~b -> a).
// Vertices are literal indices [0, 2 * num_vars).
inline bidirectional_adjacency_graph<index_domain> implication_graph(const formula_2cnf& f) {
    std::size_t num_lits = 2 * f.variable_count();
    index_domain dom(num_lits);
    std::vector<std::pair<std::size_t, std::size_t>> edges;
    edges.reserve(2 * f.clauses().size());

    for (const auto& cl : f.clauses()) {
        std::size_t a_idx = literal_index(cl.left);
        std::size_t b_idx = literal_index(cl.right);
        std::size_t not_a_idx = negated_index(a_idx);
        std::size_t not_b_idx = negated_index(b_idx);

        // ~a -> b
        edges.emplace_back(not_a_idx, b_idx);
        // ~b -> a
        edges.emplace_back(not_b_idx, a_idx);
    }

    return bidirectional_adjacency_graph<index_domain>::from_edges(dom, edges);
}

struct two_sat_result {
    bool satisfiable{false};
    std::vector<bool> assignment; // indexed by variable [0, variable_count)
};

// Linear-time 2-SAT solver using Aspvall-Plass-Tarjan algorithm:
// 1. Build directed implication graph.
// 2. Find strongly connected components (SCC).
// 3. Formula is satisfiable iff no variable x and its negation ~x belong to the same SCC.
// 4. If satisfiable, assign truth values based on topological order of SCCs.
inline two_sat_result solve_2sat(const formula_2cnf& f) {
    two_sat_result result;
    std::size_t n_vars = f.variable_count();
    if (n_vars == 0) {
        result.satisfiable = true;
        return result;
    }

    auto g = implication_graph(f);
    auto scc = algorithms::tarjan_scc(g);

    result.assignment.assign(n_vars, false);

    // Check satisfiability criterion
    for (std::size_t i = 0; i < n_vars; ++i) {
        std::size_t pos_comp = scc.component_of[2 * i];
        std::size_t neg_comp = scc.component_of[2 * i + 1];

        if (pos_comp == neg_comp) {
            result.satisfiable = false;
            result.assignment.clear();
            return result;
        }

        // Assign TRUE to the literal whose component comes later in topological order
        // (i.e. has larger component ID in our topologically normalized SCC result).
        result.assignment[i] = (pos_comp > neg_comp);
    }

    result.satisfiable = true;
    return result;
}

inline std::optional<std::vector<bool>> solve_2sat_optional(const formula_2cnf& f) {
    auto res = solve_2sat(f);
    if (!res.satisfiable) return std::nullopt;
    return res.assignment;
}

} // namespace discretex::logic
