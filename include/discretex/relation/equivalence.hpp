#pragma once

#include <cstddef>
#include <vector>
#include <span>
#include <stdexcept>
#include "../core/domain.hpp"
#include "../core/relation_concepts.hpp"
#include "../relation/dense_relation.hpp"
#include "../combinatorics/set_partitions.hpp"

namespace discretex {

// Verifies if a binary relation satisfies the equivalence relation axioms:
// 1. Reflexivity: (u, u) in R for all u in [0, n)
// 2. Symmetry: (u, v) in R ==> (v, u) in R
// 3. Transitivity: (u, v) in R and (v, w) in R ==> (u, w) in R
// Operates on any relation satisfying concepts::Relation.
template <concepts::Relation Rel>
inline bool is_equivalence_relation(const Rel& rel) {
    std::size_t n = rel.domain_size();

    // 1. Reflexivity
    for (std::size_t u = 0; u < n; ++u) {
        if (!rel.contains(u, u)) return false;
    }

    // 2. Symmetry and 3. Transitivity
    for (std::size_t u = 0; u < n; ++u) {
        for (std::size_t v = 0; v < n; ++v) {
            if (rel.contains(u, v)) {
                if (!rel.contains(v, u)) return false;
                for (std::size_t w = 0; w < n; ++w) {
                    if (rel.contains(v, w) && !rel.contains(u, w)) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

// Extracts canonical Restricted Growth String (RGS) partition from an equivalence relation.
template <concepts::Relation Rel>
inline std::vector<std::size_t> equivalence_classes_rgs(const Rel& rel) {
    std::size_t n = rel.domain_size();
    if (n == 0) return {};

    std::vector<std::size_t> rgs(n, n); // initialized to unassigned
    std::size_t current_block = 0;

    for (std::size_t u = 0; u < n; ++u) {
        if (rgs[u] == n) {
            rgs[u] = current_block;
            for (std::size_t v = u + 1; v < n; ++v) {
                if (rel.contains(u, v)) {
                    rgs[v] = current_block;
                }
            }
            ++current_block;
        }
    }
    return rgs;
}

// Extracts explicit equivalence classes (quotient set A / R) from an equivalence relation.
template <concepts::Relation Rel>
inline std::vector<std::vector<std::size_t>> equivalence_classes(const Rel& rel) {
    auto rgs = equivalence_classes_rgs(rel);
    return combinatorics::rgs_to_blocks(rgs);
}

// Returns the number of equivalence classes (the cardinality of the quotient set |A / R|).
template <concepts::Relation Rel>
inline std::size_t quotient_size(const Rel& rel) {
    auto rgs = equivalence_classes_rgs(rel);
    return combinatorics::block_count(rgs);
}

// Constructs the canonical dense equivalence relation from an RGS set partition.
template <FiniteDomain Dom>
inline dense_relation<Dom> relation_from_partition(Dom domain, std::span<const std::size_t> rgs) {
    std::size_t n = domain.size();
    if (rgs.size() != n) {
        throw std::invalid_argument("RGS size must equal domain size");
    }

    dense_relation<Dom> rel(std::move(domain));
    for (std::size_t u = 0; u < n; ++u) {
        for (std::size_t v = 0; v < n; ++v) {
            if (rgs[u] == rgs[v]) {
                rel.add_pair(u, v);
            }
        }
    }
    return rel;
}

// Constructs the canonical dense equivalence relation from explicit partition blocks.
template <FiniteDomain Dom>
inline dense_relation<Dom> relation_from_partition(Dom domain, const std::vector<std::vector<std::size_t>>& blocks) {
    std::size_t n = domain.size();
    dense_relation<Dom> rel(std::move(domain));
    for (const auto& block : blocks) {
        for (std::size_t u : block) {
            for (std::size_t v : block) {
                if (u < n && v < n) {
                    rel.add_pair(u, v);
                }
            }
        }
    }
    return rel;
}

} // namespace discretex
