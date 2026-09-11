#include <iostream>
#include <vector>
#include <discretex/discretex.hpp>

int main() {
    std::cout << "=== DiscreteX Example: Weighted Shortest Paths and Reconstruction ===\n";

    using namespace discretex;
    using namespace discretex::algorithms;

    // 1. Construct a weighted directed graph
    // Vertices: 0, 1, 2, 3, 4, 5
    weighted_directed_graph<int> g(6);
    g.add_edge(0, 1, 4);
    g.add_edge(0, 2, 2);
    g.add_edge(1, 2, 1);
    g.add_edge(1, 3, 5);
    g.add_edge(2, 3, 8);
    g.add_edge(2, 4, 10);
    g.add_edge(3, 4, 2);
    // Vertex 5 is isolated/unreachable

    std::cout << "1. Weighted graph constructed: |V| = " << g.vertex_count() 
              << ", |E| = " << g.edge_count() << "\n";

    // 2. Dijkstra Single-Source Shortest Paths from Source 0
    std::size_t source = 0;
    auto dijk_res = dijkstra_shortest_paths(g, source);

    std::cout << "2. Dijkstra distances from source 0:\n";
    for (std::size_t v = 0; v < g.vertex_count(); ++v) {
        std::cout << "     to vertex " << v << ": ";
        if (dijk_res.is_reachable(v)) {
            std::cout << "dist = " << *dijk_res.distance[v] << "\n";
        } else {
            std::cout << "UNREACHABLE\n";
        }
    }

    // 3. Lazy Path Reconstruction and Integrity Verification
    std::size_t target = 4;
    auto path = reconstruct_path(dijk_res, target);
    if (path.has_value()) {
        std::cout << "3. Reconstructed shortest path to vertex " << target << ": ";
        for (std::size_t i = 0; i < path->size(); ++i) {
            std::cout << (*path)[i] << (i + 1 < path->size() ? " -> " : "\n");
        }
        bool valid = verify_path_integrity(g, *path, *dijk_res.distance[target]);
        std::cout << "   Path integrity verification: " << (valid ? "VERIFIED" : "FAILED") << "\n";
    }

    // 4. Floyd-Warshall All-Pairs Shortest Paths
    auto fw_res = floyd_warshall_all_pairs(g);
    std::cout << "4. Floyd-Warshall All-Pairs Distance Matrix:\n";
    std::cout << "       ";
    for (std::size_t v = 0; v < g.vertex_count(); ++v) std::cout << v << "   ";
    std::cout << "\n";
    for (std::size_t u = 0; u < g.vertex_count(); ++u) {
        std::cout << "  " << u << " | ";
        for (std::size_t v = 0; v < g.vertex_count(); ++v) {
            if (fw_res.distance[u][v].has_value()) {
                std::cout << *fw_res.distance[u][v] << "   ";
            } else {
                std::cout << "inf ";
            }
        }
        std::cout << "\n";
    }

    return 0;
}
