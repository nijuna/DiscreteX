#pragma once

#include <cstddef>
#include <vector>
#include <utility>
#include "tarjan_scc.hpp"
#include "../core/domain.hpp"
#include "../graph/sparse_graph.hpp"

namespace discretex::algorithms {

// Constructs the condensation DAG where each strongly connected component is contracted
// into a single meta-vertex.
// The resulting graph has domain index_domain(scc.component_count) and is guaranteed acyclic.
template <concepts::ForwardRelation G>
inline bidirectional_adjacency_graph<index_domain> condensation_dag(const G& g, const scc_result& scc) {
    index_domain dom(scc.component_count);
    std::vector<std::pair<std::size_t, std::size_t>> edges;

    std::size_t n = g.domain_size();
    for (std::size_t u = 0; u < n; ++u) {
        std::size_t cu = scc.component_of[u];
        for (std::size_t v : g.out_neighbors(u)) {
            std::size_t cv = scc.component_of[v];
            if (cu != cv) {
                edges.emplace_back(cu, cv);
            }
        }
    }

    return bidirectional_adjacency_graph<index_domain>::from_edges(dom, edges);
}

} // namespace discretex::algorithms
