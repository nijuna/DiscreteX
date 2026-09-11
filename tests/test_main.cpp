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
    assert(!rel.contains(3, 0));
    std::cout << "  -> Passed\n";
}

void test_sparse_graphs() {
    std::cout << "[Test] Sparse Graphs (Forward & Bidirectional with from_edges)...\n";
    index_domain dom(5);

    std::vector<std::pair<std::size_t, std::size_t>> edge_list = {
        {0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 4}, {0, 1}
    };

    auto g = bidirectional_adjacency_graph<index_domain>::from_edges(dom, edge_list);
    assert(g.domain_size() == 5);
    assert(g.edge_count() == 5);

    assert(g.contains(0, 1));
    assert(g.contains(0, 2));
    assert(!g.contains(1, 0));

    auto out_0 = g.out_neighbors(0);
    assert((std::vector<std::size_t>(out_0.begin(), out_0.end()) == std::vector<std::size_t>{1, 2}));

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

    auto out_1 = g_transposed.out_neighbors(1);
    assert((std::vector<std::size_t>(out_1.begin(), out_1.end()) == std::vector<std::size_t>{0}));
    std::cout << "  -> Passed\n";
}

void test_unified_bfs() {
    std::cout << "[Test] Unified BFS across Dense, Sparse, and Transpose Views...\n";
    index_domain dom(4);

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

    auto g_rev = views::transpose(g_sparse);
    auto bfs_rev = bfs_order(g_rev, 3);
    assert((bfs_rev == std::vector<std::size_t>{3, 2, 0, 1}));
    std::cout << "  -> Passed\n";
}

void test_combinatorics() {
    std::cout << "[Test] Combinatorics (k_subsets, power_set, projection)...\n";

    // k_subsets: combinations(4, 2) = 6
    std::vector<std::vector<std::size_t>> c4_2;
    for (auto comb : views::k_subsets(4, 2)) {
        c4_2.emplace_back(comb.begin(), comb.end());
    }
    assert(c4_2.size() == 6);
    std::vector<std::vector<std::size_t>> expected_c4_2 = {
        {0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}
    };
    assert(c4_2 == expected_c4_2);

    // power_set binary: 2^3 = 8
    std::vector<std::vector<std::size_t>> p3_binary;
    for (auto subset : views::power_set(3)) {
        p3_binary.emplace_back(subset.begin(), subset.end());
    }
    assert(p3_binary.size() == 8);

    // power_set gray code: 2^3 = 8
    std::vector<std::vector<std::size_t>> p3_gray;
    for (auto subset : views::power_set(3, views::subset_ordering::gray_code)) {
        p3_gray.emplace_back(subset.begin(), subset.end());
    }
    assert(p3_gray.size() == 8);

    // Domain projection
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

void test_poset_and_boolean_lattice() {
    std::cout << "[Test] Order Theory: Posets, Hasse Diagrams & Boolean Lattice B3...\n";

    // 1. Basic Poset from DAG
    // DAG: 0 -> 1, 1 -> 2, 0 -> 2
    index_domain dom3(3);
    bidirectional_adjacency_graph g_dag(dom3);
    g_dag.add_edge(0, 1);
    g_dag.add_edge(1, 2);
    g_dag.add_edge(0, 2); // Transitive edge

    auto p = poset<index_domain>::from_dag_closure(g_dag);
    assert(p.less_equal(0, 0));
    assert(p.less_equal(0, 2));
    assert(p.less(0, 2));
    assert(!p.less_equal(2, 0));

    // Hasse diagram should have transitive edge (0, 2) removed
    auto hasse = p.hasse_diagram();
    assert(hasse.edge_count() == 2);
    assert(hasse.contains(0, 1));
    assert(hasse.contains(1, 2));
    assert(!hasse.contains(0, 2)); // Reduced!

    // Linear extension
    auto lin_ext = p.linear_extension();
    assert((lin_ext == std::vector<std::size_t>{0, 1, 2}));

    // 2. The Golden Integration Test: Boolean Lattice B3 = (P({0, 1, 2}), <=)
    // Vertices are bitmasks 0 to 7. Subset inclusion: A <= B <==> (A | B) == B.
    std::size_t n_bits = 3;
    std::size_t num_subsets = 1ULL << n_bits; // 8
    index_domain lattice_dom(num_subsets);

    dense_relation lattice_rel(lattice_dom);
    for (std::size_t u = 0; u < num_subsets; ++u) {
        for (std::size_t v = 0; v < num_subsets; ++v) {
            if ((u | v) == v) { // u is subset of v
                lattice_rel.add_pair(u, v);
            }
        }
    }

    poset boolean_lattice(std::move(lattice_rel));

    // A. Verify it is a lattice
    assert(boolean_lattice.is_lattice());

    // B. Verify join (A | B) and meet (A & B) match bitwise operations
    for (std::size_t u = 0; u < num_subsets; ++u) {
        for (std::size_t v = 0; v < num_subsets; ++v) {
            auto j = boolean_lattice.join(u, v);
            auto m = boolean_lattice.meet(u, v);
            assert(j.has_value() && *j == (u | v));
            assert(m.has_value() && *m == (u & v));
        }
    }

    // C. Verify Hasse Diagram is Hypercube Q3:
    // Vertices = 8, Edges = 3 * 2^(3-1) = 12 edges
    auto q3_hasse = boolean_lattice.hasse_diagram();
    assert(q3_hasse.edge_count() == 12);

    // Each edge in Hasse diagram should be adding exactly 1 bit
    for (std::size_t u = 0; u < num_subsets; ++u) {
        for (std::size_t v : q3_hasse.out_neighbors(u)) {
            // v must have popcount(u) + 1
            assert(std::popcount(v) == std::popcount(u) + 1);
            assert((u | v) == v);
        }
    }

    // D. Linear extension: empty set (0) must be first, full set (7) must be last
    auto b3_order = boolean_lattice.linear_extension();
    assert(b3_order.front() == 0);
    assert(b3_order.back() == 7);

    // 3. Test from_hasse_diagram rejection of transitive shortcuts
    bool threw = false;
    try {
        // g_dag has edge (0, 2), which is NOT a covering relation
        poset<index_domain>::from_hasse_diagram(g_dag);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw); // Successfully caught and rejected unreduced diagram

    std::cout << "  -> Passed (Boolean Lattice B3 verified with Hypercube Q3 Hasse diagram)\n";
}

void test_propositional_logic() {
    std::cout << "[Test] Propositional Logic: AST, Truth Tables & Normal Forms...\n";
    using namespace discretex::logic;

    auto p = var(0);
    auto q = var(1);
    auto r = var(2);

    // 1. Evaluation & Operator Overloads
    auto f1 = p && q;
    assert(evaluate(f1, 0b00) == false); // p=0, q=0
    assert(evaluate(f1, 0b01) == false); // p=1, q=0
    assert(evaluate(f1, 0b10) == false); // p=0, q=1
    assert(evaluate(f1, 0b11) == true);  // p=1, q=1

    auto f2 = p || !q;
    assert(evaluate(f2, 0b00) == true);
    assert(evaluate(f2, 0b10) == false); // p=0, q=1 -> 0 || 0 = 0
    assert(evaluate(f2, 0b01) == true);
    assert(evaluate(f2, 0b11) == true);

    // 2. Truth Tables & Semantic Classification
    // Law of Excluded Middle: p | ~p is a tautology
    auto lem = p || !p;
    assert(is_tautology(lem));
    assert(!is_contradiction(lem));
    assert(is_satisfiable(lem));

    // Modus Ponens valid implication: ((p -> q) & p) -> q
    auto mp = implies_(implies_(p, q) && p, q);
    assert(is_tautology(mp));

    // Contradiction: p & ~p
    auto contra = p && !p;
    assert(is_contradiction(contra));
    assert(!is_tautology(contra));
    assert(!is_satisfiable(contra));

    // Equivalence: p -> q <=> ~p | q
    auto imp_form = implies_(p, q);
    auto disj_form = !p || q;
    assert(equivalent(imp_form, disj_form));

    // De Morgan Equivalence: ~(p & q) <=> ~p | ~q
    auto de_morgan_lhs = !(p && q);
    auto de_morgan_rhs = !p || !q;
    assert(equivalent(de_morgan_lhs, de_morgan_rhs));

    // Entailment: (p & q) |= p
    assert(entails(p && q, p));
    assert(!entails(p, p && q));
    assert(entails(p, p || q));

    // 3. Normal Forms (NNF, DNF, CNF)
    auto complex_formula = iff_(p && implies_(q, r), !(p || !r));

    // NNF preserves equivalence
    auto nnf_form = to_nnf(complex_formula);
    assert(equivalent(complex_formula, nnf_form));

    // Algebraic DNF & CNF preserve equivalence
    auto dnf_form = to_dnf(complex_formula);
    assert(equivalent(complex_formula, dnf_form));

    auto cnf_form = to_cnf(complex_formula);
    assert(equivalent(complex_formula, cnf_form));

    // Canonical DNF & CNF from truth table preserve equivalence
    auto c_dnf = to_canonical_dnf(complex_formula);
    assert(equivalent(complex_formula, c_dnf));

    auto c_cnf = to_canonical_cnf(complex_formula);
    assert(equivalent(complex_formula, c_cnf));

    // Test simplification in DNF/CNF:
    // (p & ~p) in DNF simplifies to false
    assert(to_dnf(p && !p) == boolean(false));
    // (p | ~p) in CNF simplifies to true
    assert(to_cnf(p || !p) == boolean(true));

    std::cout << "  -> Passed (Evaluation, Tautologies, Equivalences, NNF, DNF, CNF)\n";
}

void test_valuation_space_bridge() {
    std::cout << "[Test] Bridge: Valuation Algebra & Boolean Lattice Isomorphism...\n";
    using namespace discretex::logic;

    // 3 variables: p=x0, q=x1, r=x2 -> 8 valuations [0..7]
    std::size_t num_vars = 3;
    std::size_t num_valuations = 1ULL << num_vars; // 8

    auto p = var(0);
    auto q = var(1);
    auto r = var(2);

    auto f = p || q;
    auto g = q && r;

    truth_table tt_f(f, num_vars);
    truth_table tt_g(g, num_vars);
    truth_table tt_and(f && g, num_vars);
    truth_table tt_or(f || g, num_vars);
    truth_table tt_not(!f, num_vars);

    auto sat_f = tt_f.satisfying_assignments();
    auto sat_g = tt_g.satisfying_assignments();
    auto sat_and = tt_and.satisfying_assignments();
    auto sat_or = tt_or.satisfying_assignments();
    auto sat_not = tt_not.satisfying_assignments();

    // Convert to bitmasks representing subsets of valuation space
    auto to_val_mask = [](const std::vector<std::size_t>& assignments) {
        uint64_t mask = 0;
        for (std::size_t a : assignments) {
            mask |= (1ULL << a);
        }
        return mask;
    };

    uint64_t mask_f = to_val_mask(sat_f);
    uint64_t mask_g = to_val_mask(sat_g);
    uint64_t mask_and = to_val_mask(sat_and);
    uint64_t mask_or = to_val_mask(sat_or);
    uint64_t mask_not = to_val_mask(sat_not);

    // 1. Meet corresponds to bitwise intersection of valuations
    assert(mask_and == (mask_f & mask_g));

    // 2. Join corresponds to bitwise union of valuations
    assert(mask_or == (mask_f | mask_g));

    // 3. Complement corresponds to relative complement in V_n
    uint64_t universe_mask = (1ULL << num_valuations) - 1ULL;
    assert(mask_not == (~mask_f & universe_mask));

    // 4. Entailment corresponds to subset inclusion in the Boolean lattice
    assert(entails(f && g, f));
    assert((mask_and | mask_f) == mask_f); // Sat(f & g) subset of Sat(f)

    // 5. Connect to poset: Poset of valuations under inclusion is a lattice
    index_domain val_dom(num_valuations);
    dense_relation val_rel(val_dom);
    for (std::size_t u = 0; u < num_valuations; ++u) {
        for (std::size_t v = 0; v < num_valuations; ++v) {
            if ((u | v) == v) {
                val_rel.add_pair(u, v);
            }
        }
    }
    poset val_lattice(std::move(val_rel));
    assert(val_lattice.is_lattice());

    std::cout << "  -> Passed (Valuation subsets isomorphic to Boolean Lattice operations)\n";
}

void test_number_theory() {
    std::cout << "[Test] Number Theory: Euclidean, Modular Rings, Primes & CRT...\n";
    using namespace discretex::number_theory;

    // 1. Extended GCD
    auto eg = extended_gcd<int64_t>(240, 46);
    assert(eg.gcd == 2);
    assert(240 * eg.x + 46 * eg.y == eg.gcd);

    // Negative input handling
    auto eg_neg = extended_gcd<int64_t>(-35, 15);
    assert(eg_neg.gcd == 5);
    assert(-35 * eg_neg.x + 15 * eg_neg.y == 5);

    // 2. Linear Diophantine Equation: 12x + 18y = 30
    auto dio = solve_diophantine<int64_t>(12, 18, 30);
    assert(dio.has_solution);
    assert(dio.gcd == 6);
    assert(12 * dio.x0 + 18 * dio.y0 == 30);

    auto dio_nosol = solve_diophantine<int64_t>(12, 18, 31);
    assert(!dio_nosol.has_solution);

    // 3. Modular Arithmetic & Inverse
    assert(add_mod(10, 20, 25) == 5);
    assert(sub_mod(5, 10, 25) == 20);
    assert(mul_mod(10, 20, 25) == 0);
    assert(mul_mod(7, 8, 25) == 6);

    // Fermat's Little Theorem: 2^(p-1) = 1 (mod p)
    uint64_t p_mod = 1000000007ULL;
    assert(power_mod(2, p_mod - 1, p_mod) == 1);

    // Modular Inverse
    auto inv3 = mod_inverse(3, 11);
    assert(inv3.has_value() && *inv3 == 4); // 3 * 4 = 12 = 1 (mod 11)
    auto inv_none = mod_inverse(6, 9);
    assert(!inv_none.has_value());

    // Dynamic mod int
    dynamic_mod_int a(7, 13);
    dynamic_mod_int b(8, 13);
    assert((a + b).value() == 2); // 15 mod 13
    assert((a * b).value() == 4); // 56 mod 13
    assert((a / b * b).value() == 7);
    assert(a.pow(12).value() == 1); // FLT

    // 4. Primality (Deterministic Miller-Rabin)
    assert(!is_prime(0));
    assert(!is_prime(1));
    assert(is_prime(2));
    assert(is_prime(3));
    assert(is_prime(5));
    assert(is_prime(104729)); // 10,000th prime
    assert(is_prime(1000000007ULL));

    assert(!is_prime(4));
    assert(!is_prime(9));
    assert(!is_prime(561)); // Carmichael number 3 * 11 * 17
    assert(!is_prime(1105)); // Carmichael number 5 * 13 * 17

    // Sieve of Eratosthenes
    auto primes_50 = sieve_of_eratosthenes(50);
    assert(primes_50.size() == 15);
    assert(primes_50.front() == 2);
    assert(primes_50.back() == 47);

    // Prime Factorization
    auto factors_360 = prime_factors(360); // 2^3 * 3^2 * 5^1
    assert(factors_360.size() == 3);
    assert((factors_360[0] == std::pair<uint64_t, std::size_t>{2, 3}));
    assert((factors_360[1] == std::pair<uint64_t, std::size_t>{3, 2}));
    assert((factors_360[2] == std::pair<uint64_t, std::size_t>{5, 1}));

    // Divisors
    auto divs_12 = divisors(12);
    assert((divs_12 == std::vector<uint64_t>{1, 2, 3, 4, 6, 12}));

    // Euler Totient & Carmichael
    assert(euler_totient(1) == 1);
    assert(euler_totient(9) == 6);
    assert(euler_totient(12) == 4);
    assert(euler_totient(13) == 12);
    assert(carmichael(8) == 2);
    assert(carmichael(12) == 2);

    // 5. Linear Congruence: 6x = 9 (mod 15) -> solutions {4, 9, 14}
    auto cong_sols = solve_linear_congruence(6, 9, 15);
    assert((cong_sols == std::vector<uint64_t>{4, 9, 14}));

    // 6. Chinese Remainder Theorem
    // Sunzi's problem: x = 2 (mod 3), x = 3 (mod 5), x = 2 (mod 7) -> x = 23 (mod 105)
    std::vector<congruence> sunzi = {
        {2, 3}, {3, 5}, {2, 7}
    };
    auto crt_sunzi = chinese_remainder_theorem(sunzi);
    assert(crt_sunzi.has_value());
    assert(crt_sunzi->first == 23);
    assert(crt_sunzi->second == 105);

    // Non-coprime consistent: x = 2 (mod 4), x = 4 (mod 6) -> x = 10 (mod 12)
    std::vector<congruence> non_coprime_ok = {
        {2, 4}, {4, 6}
    };
    auto crt_nc = chinese_remainder_theorem(non_coprime_ok);
    assert(crt_nc.has_value());
    assert(crt_nc->first == 10);
    assert(crt_nc->second == 12);

    // Non-coprime inconsistent: x = 1 (mod 4), x = 2 (mod 6) -> no solution
    std::vector<congruence> non_coprime_bad = {
        {1, 4}, {2, 6}
    };
    auto crt_bad = chinese_remainder_theorem(non_coprime_bad);
    assert(!crt_bad.has_value());

    std::cout << "  -> Passed (Extended GCD, Diophantine, Modular Rings, Miller-Rabin, Sieve, CRT)\n";
}

void test_divisibility_lattice_bridge() {
    std::cout << "[Test] Bridge: Divisibility Lattice D_30 Isomorphic to Boolean Lattice B_3...\n";
    using namespace discretex::number_theory;

    // Divisors of 30 = 2 * 3 * 5: {1, 2, 3, 5, 6, 10, 15, 30}
    auto divs = divisors(30);
    assert(divs.size() == 8);

    mapped_domain<uint64_t> div_dom(divs);
    dense_relation rel(div_dom);

    // Order: u <= v <=> u divides v
    for (uint64_t u : divs) {
        for (uint64_t v : divs) {
            if (v % u == 0) {
                rel.add_pair(div_dom.to_index(u), div_dom.to_index(v));
            }
        }
    }

    poset d30_lattice(std::move(rel));

    // 1. Verify D_30 is a lattice
    assert(d30_lattice.is_lattice());

    // 2. Verify meet is gcd and join is lcm
    for (uint64_t u : divs) {
        for (uint64_t v : divs) {
            std::size_t u_idx = div_dom.to_index(u);
            std::size_t v_idx = div_dom.to_index(v);

            auto m_idx = d30_lattice.meet(u_idx, v_idx);
            auto j_idx = d30_lattice.join(u_idx, v_idx);

            assert(m_idx.has_value());
            assert(j_idx.has_value());

            assert(div_dom.from_index(*m_idx) == std::gcd(u, v));
            assert(div_dom.from_index(*j_idx) == std::lcm(u, v));
        }
    }

    // 3. Verify Hasse diagram has 12 edges (isomorphic to 3-cube Q3)
    auto hasse = d30_lattice.hasse_diagram();
    assert(hasse.edge_count() == 12);

    // Each covering relation u -< v must correspond to multiplying by a prime factor
    for (std::size_t u_idx = 0; u_idx < div_dom.size(); ++u_idx) {
        uint64_t u_val = div_dom.from_index(u_idx);
        for (std::size_t v_idx : hasse.out_neighbors(u_idx)) {
            uint64_t v_val = div_dom.from_index(v_idx);
            assert(v_val % u_val == 0);
            uint64_t quotient = v_val / u_val;
            // Quotient must be prime (2, 3, or 5)
            assert(quotient == 2 || quotient == 3 || quotient == 5);
        }
    }

    // 4. Linear extension: bottom is 1, top is 30
    auto order = d30_lattice.linear_extension();
    assert(div_dom.from_index(order.front()) == 1);
    assert(div_dom.from_index(order.back()) == 30);

    std::cout << "  -> Passed (Divisibility Lattice D_30 satisfies lattice axioms & matches Q_3 Hasse diagram)\n";
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
    test_poset_and_boolean_lattice();
    test_propositional_logic();
    test_valuation_space_bridge();
    test_number_theory();
    test_divisibility_lattice_bridge();

    std::cout << "\nALL TESTS PASSED SUCCESSFULLY (100%)\n";
    return 0;
}


