#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>
#include "../core/relation_concepts.hpp"

namespace discretex::algorithms {

struct scc_result {
    std::size_t component_count{0};
    std::vector<std::size_t> component_of; // vertex -> component ID [0, component_count)
    std::vector<std::vector<std::size_t>> components; // component ID -> vertices in that component
};

// Tarjan Strongly Connected Components (SCC) algorithm.
// Operates on any directed graph or relation satisfying concepts::ForwardRelation.
// Components are normalized in topological order of the condensation DAG:
// if there is a directed path from component A to component B (A != B), then A < B.
template <concepts::ForwardRelation G>
inline scc_result tarjan_scc(const G& g) {
    std::size_t n = g.domain_size();
    scc_result result;
    result.component_of.assign(n, 0);

    if (n == 0) return result;

    std::vector<std::size_t> dfn(n, 0);
    std::vector<std::size_t> low(n, 0);
    std::vector<bool> in_stack(n, false);
    std::vector<std::size_t> st;
    st.reserve(n);

    std::vector<std::vector<std::size_t>> raw_components;
    std::size_t timer = 1;

    auto dfs = [&](auto& self, std::size_t u) -> void {
        dfn[u] = low[u] = timer++;
        st.push_back(u);
        in_stack[u] = true;

        for (std::size_t v : g.out_neighbors(u)) {
            if (dfn[v] == 0) {
                self(self, v);
                low[u] = std::min(low[u], low[v]);
            } else if (in_stack[v]) {
                low[u] = std::min(low[u], dfn[v]);
            }
        }

        if (low[u] == dfn[u]) {
            std::vector<std::size_t> comp;
            while (true) {
                std::size_t v = st.back();
                st.pop_back();
                in_stack[v] = false;
                comp.push_back(v);
                if (v == u) break;
            }
            raw_components.push_back(std::move(comp));
        }
    };

    for (std::size_t u = 0; u < n; ++u) {
        if (dfn[u] == 0) {
            dfs(dfs, u);
        }
    }

    // Tarjan discovers components in reverse topological order (sinks first).
    // Reversing gives standard topological order (sources first, sinks last).
    std::reverse(raw_components.begin(), raw_components.end());
    result.component_count = raw_components.size();
    result.components = std::move(raw_components);

    for (std::size_t c = 0; c < result.component_count; ++c) {
        for (std::size_t v : result.components[c]) {
            result.component_of[v] = c;
        }
    }

    return result;
}

} // namespace discretex::algorithms
