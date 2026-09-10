#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include "../include/discretex/discretex.hpp"

using namespace discretex;

void test_domains() {
    std::cout << "[Test] Domains (index_domain & mapped_domain)...\n";
    index_domain idx_dom(5);
    assert(idx_dom.size() == 5);
    assert(idx_dom.to_index(3) == 3);
    assert(idx_dom.from_index(3) == 3);
    assert(idx_dom.contains(4));
    assert(!idx_dom.contains(5));

    mapped_domain<std::string> names{"Alice", "Bob", "Charlie"};
    assert(names.size() == 3);
    assert(names.to_index("Alice") == 0);
    assert(names.to_index("Bob") == 1);
    assert(names.to_index("Charlie") == 2);
    assert(names.from_index(1) == "Bob");
    assert(names.contains("Alice"));
    assert(!names.contains("David"));
    std::cout << "  -> Passed\n";
}

void test_dense_relation() {
    std::cout << "[Test] Dense Relation (Bit-matrix & Warshall closure)...\n";
    index_domain dom(4);
    dense_relation rel(dom);

    // 0 -> 1, 1 -> 2, 2 -> 3
    rel.add_pair(0, 1);
    rel.add_pair(1, 2);
    rel.add_pair(2, 3);

    assert(rel.contains(0, 1));
    assert(rel.contains(1, 2));
    assert(!rel.contains(0, 2));

    // Test bit_row_fiber_view
    std::vector<std::size_t> neighbors_0;
    for (std::size_t v : rel.out_neighbors(0)) {
        neighbors_0.push_back(v);
    }
    assert((neighbors_0 == std::vector<std::size_t>{1}));

    // Test in-place Warshall closure
    rel.transitive_closure_inplace();
    assert(rel.contains(0, 1));
    assert(rel.contains(0, 2));
    assert(rel.contains(0, 3));
    assert(!rel.contains(3, 0)); // DAG remains antisymmetric
    std::cout << "  -> Passed\n";
}

void test_sparse_graphs() {
    std::cout << "[Test] Sparse Graphs (Forward & Bidirectional with from_edges)...\n";
    index_domain dom(5);

    // Test bulk loading with from_edges
    std::vector<std::pair<std::size_t, std::size_t>> edge_list = {
        {0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 4}, {0, 1} // Duplicate (0, 1)
    };

    auto g = bidirectional_adjacency_graph<index_domain>::from_edges(dom, edge_list);
    assert(g.domain_size() == 5);
    assert(g.edge_count() == 5); // Duplicate was filtered out

    assert(g.contains(0, 1));
    assert(g.contains(0, 2));
    assert(!g.contains(1, 0)); // Directed

    // Check out-neighbors
    auto out_0 = g.out_neighbors(0);
    assert((std::vector<std::size_t>(out_0.begin(), out_0.end()) == std::vector<std::size_t>{1, 2}));

    // Check in-neighbors
    auto in_3 = g.in_neighbors(3);
    assert((std::vector<std::size_t>(in_3.begin(), in_3.end()) == std::vector<std::size_t>{1, 2}));
    assert(g.in_degree(3) == 2);
    assert(g.out_degree(3) == 1);
    std::cout << "  -> Passed\n";
}

void test_transpose_view() {
    std::cout << "[Test] Transpose View (Zero-copy converse)...\n";
    index_domain dom(3);
    bidirectional_adjacency_graph g(dom);
    g.add_edge(0, 1);
    g.add_edge(1, 2);

    auto g_transposed = views::transpose(g);
    assert(g_transposed.domain_size() == 3);
    assert(g_transposed.contains(1, 0));
    assert(g_transposed.contains(2, 1));
    assert(!g_transposed.contains(0, 1));

    // Out-neighbors of transpose should be in-neighbors of original
    auto out_1 = g_transposed.out_neighbors(1);
    assert((std::vector<std::size_t>(out_1.begin(), out_1.end()) == std::vector<std::size_t>{0}));
    std::cout << "  -> Passed\n";
}

void test_unified_bfs() {
    std::cout << "[Test] Unified BFS across Dense, Sparse, and Transpose Views...\n";
    index_domain dom(4);

    // Graph: 0 -> 1 -> 2 -> 3 and 0 -> 2
    dense_relation r_dense(dom, {{0, 1}, {0, 2}, {1, 2}, {2, 3}});
    bidirectional_adjacency_graph g_sparse(dom);
    g_sparse.add_edge(0, 1);
    g_sparse.add_edge(0, 2);
    g_sparse.add_edge(1, 2);
    g_sparse.add_edge(2, 3);

    auto bfs_dense = bfs_order(r_dense, 0);
    auto bfs_sparse = bfs_order(g_sparse, 0);

    assert(bfs_dense == bfs_sparse);
    assert((bfs_dense == std::vector<std::size_t>{0, 1, 2, 3}));

    // BFS on Transpose of Sparse: starting from 3 should reach {3, 2, 0, 1}
    auto g_rev = views::transpose(g_sparse);
    auto bfs_rev = bfs_order(g_rev, 3);
    assert((bfs_rev == std::vector<std::size_t>{3, 2, 0, 1}));
    std::cout << "  -> Passed\n";
}

void test_combinatorics() {
    std::cout << "[Test] Combinatorics (k_subsets, power_set, projection)...\n";

    // 1. k_subsets: combinations(4, 2) = 6
    std::vector<std::vector<std::size_t>> c4_2;
    for (auto comb : views::k_subsets(4, 2)) {
        c4_2.emplace_back(comb.begin(), comb.end());
    }
    assert(c4_2.size() == 6);
    std::vector<std::vector<std::size_t>> expected_c4_2 = {
        {0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}
    };
    assert(c4_2 == expected_c4_2);

    // 2. power_set binary: 2^3 = 8
    std::vector<std::vector<std::size_t>> p3_binary;
    for (auto subset : views::power_set(3)) {
        p3_binary.emplace_back(subset.begin(), subset.end());
    }
    assert(p3_binary.size() == 8);

    // 3. power_set gray code: 2^3 = 8, consecutive subsets differ by 1 element
    std::vector<std::vector<std::size_t>> p3_gray;
    for (auto subset : views::power_set(3, views::subset_ordering::gray_code)) {
        p3_gray.emplace_back(subset.begin(), subset.end());
    }
    assert(p3_gray.size() == 8);

    // 4. Domain projection: mapped_domain with k_subsets
    mapped_domain<std::string> fruits{"Apple", "Banana", "Cherry"};
    std::vector<std::vector<std::string>> projected;
    for (auto subset : views::project_subsets(fruits, views::k_subsets(3, 2))) {
        std::vector<std::string> group;
        for (const auto& item : subset) {
            group.push_back(item);
        }
        projected.push_back(group);
    }
    assert(projected.size() == 3);
    assert((projected[0] == std::vector<std::string>{"Apple", "Banana"}));
    assert((projected[1] == std::vector<std::string>{"Apple", "Cherry"}));
    assert((projected[2] == std::vector<std::string>{"Banana", "Cherry"}));

    std::cout << "  -> Passed\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  DiscreteX Core Test Suite (C++20)     \n";
    std::cout << "========================================\n";

    test_domains();
    test_dense_relation();
    test_sparse_graphs();
    test_transpose_view();
    test_unified_bfs();
    test_combinatorics();

    std::cout << "\nALL TESTS PASSED SUCCESSFULLY (100%)\n";
    return 0;
}
