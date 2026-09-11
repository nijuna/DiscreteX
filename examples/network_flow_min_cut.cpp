#include <iostream>
#include <vector>
#include <discretex/discretex.hpp>

int main() {
    std::cout << "=== DiscreteX Example: Maximum Flow and Minimum Cut Duality ===\n";

    using namespace discretex;
    using namespace discretex::algorithms;

    // 1. Construct flow network with 6 vertices
    // 0: Source (S), 5: Sink (T)
    flow_network<int> net(6);
    net.add_edge(0, 1, 16);
    net.add_edge(0, 2, 13);
    net.add_edge(1, 2, 10);
    net.add_edge(1, 3, 12);
    net.add_edge(2, 1, 4);
    net.add_edge(2, 4, 14);
    net.add_edge(3, 2, 9);
    net.add_edge(3, 5, 20);
    net.add_edge(4, 3, 7);
    net.add_edge(4, 5, 4);

    std::cout << "1. Flow network constructed: |V| = " << net.vertex_count() 
              << ", |E| = " << net.edge_count() << "\n";

    // 2. Solve Max-Flow via Dinic blocking flow algorithm
    std::size_t source = 0;
    std::size_t sink = 5;
    auto dinic_res = max_flow_dinic(net, source, sink);

    std::cout << "2. Dinic algorithm completed: Maximum Flow = " << dinic_res.max_flow << "\n";

    // 3. Verify Flow Conservation
    bool conserved = verify_flow_conservation(net, dinic_res, source, sink);
    std::cout << "3. Flow conservation at all intermediate vertices: " 
              << (conserved ? "VERIFIED" : "VIOLATED") << "\n";

    // 4. Max-Flow Min-Cut Theorem Verification
    int cut_capacity = compute_cut_capacity(net, dinic_res.source_side_min_cut);
    std::cout << "4. Minimum Cut Capacity = " << cut_capacity << "\n";
    std::cout << "   Max-Flow == Min-Cut Duality: " 
              << (dinic_res.max_flow == cut_capacity ? "CERTIFIED" : "FAILED") << "\n";

    std::cout << "   Source-side cut partition S: { ";
    for (std::size_t u = 0; u < net.vertex_count(); ++u) {
        if (dinic_res.source_side_min_cut[u]) std::cout << u << " ";
    }
    std::cout << "}\n";

    std::cout << "   Sink-side cut partition T: { ";
    for (std::size_t u = 0; u < net.vertex_count(); ++u) {
        if (!dinic_res.source_side_min_cut[u]) std::cout << u << " ";
    }
    std::cout << "}\n";

    return 0;
}
