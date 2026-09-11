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

void test_counting_functions() {
    std::cout << "[Test] Enumerative Combinatorics: Counting Sequences & Exact Numbers...\n";
    using namespace discretex::combinatorics;

    // 1. Factorial & Falling Factorial
    assert(factorial(0) == 1);
    assert(factorial(1) == 1);
    assert(factorial(5) == 120);
    assert(factorial(10) == 3628800);
    assert(factorial(20) == 2432902008176640000ULL);

    bool threw_fact = false;
    try {
        factorial(21);
    } catch (const std::overflow_error&) {
        threw_fact = true;
    }
    assert(threw_fact);

    assert(falling_factorial(5, 2) == 20); // 5 * 4
    assert(falling_factorial(5, 0) == 1);
    assert(falling_factorial(5, 5) == 120);
    assert(falling_factorial(5, 6) == 0);

    // 2. Combinations (Binomial Coefficients)
    assert(combinations_count(5, 2) == 10);
    assert(combinations_count(10, 4) == 210);
    assert(combinations_count(50, 5) == 2118760);
    assert(combinations_count(5, 6) == 0);

    // 3. Stirling Numbers of the Second Kind S(n, k)
    assert(stirling_second(0, 0) == 1);
    assert(stirling_second(4, 0) == 0);
    assert(stirling_second(4, 1) == 1);
    assert(stirling_second(4, 2) == 7);
    assert(stirling_second(4, 3) == 6);
    assert(stirling_second(4, 4) == 1);
    assert(stirling_second(5, 3) == 25);

    // 4. Unsigned Stirling Numbers of the First Kind |c(n, k)|
    assert(stirling_first_unsigned(4, 2) == 11);
    assert(stirling_first_unsigned(5, 3) == 35);
    assert(stirling_first(4, 2) == 11);

    // 5. Bell Numbers B(n)
    assert(bell_number(0) == 1);
    assert(bell_number(1) == 1);
    assert(bell_number(2) == 2);
    assert(bell_number(3) == 5);
    assert(bell_number(4) == 15);
    assert(bell_number(5) == 52);
    assert(bell_number(6) == 203);

    // Verify Bell number is sum of Stirling numbers of the second kind: B(4) = sum S(4, k)
    uint64_t b4_sum = stirling_second(4, 0) + stirling_second(4, 1) + stirling_second(4, 2) +
                      stirling_second(4, 3) + stirling_second(4, 4);
    assert(b4_sum == bell_number(4));

    // 6. Catalan Numbers C_n
    assert(catalan_number(0) == 1);
    assert(catalan_number(1) == 1);
    assert(catalan_number(2) == 2);
    assert(catalan_number(3) == 5);
    assert(catalan_number(4) == 14);
    assert(catalan_number(5) == 42);

    // 7. Partition Numbers p(n)
    assert(partition_number(0) == 1);
    assert(partition_number(1) == 1);
    assert(partition_number(2) == 2);
    assert(partition_number(3) == 3);
    assert(partition_number(4) == 5);
    assert(partition_number(5) == 7);
    assert(partition_number(10) == 42);

    std::cout << "  -> Passed (Factorial, Falling Factorial, Combinations, Stirling, Bell, Catalan, Partition)\n";
}

void test_permutations_and_integer_partitions() {
    std::cout << "[Test] Enumerative Combinatorics: Permutations & Integer Partitions Views...\n";
    using namespace discretex::combinatorics;

    // 1. Permutations View of 3 elements: 3! = 6
    permutations_view p3(3);
    assert(p3.size() == 6);

    std::vector<std::vector<std::size_t>> generated_perms;
    for (auto p : p3) {
        generated_perms.emplace_back(p.begin(), p.end());
    }

    std::vector<std::vector<std::size_t>> expected_perms = {
        {0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}
    };
    assert(generated_perms == expected_perms);

    // 2. Permutation projection onto mapped domain
    mapped_domain<std::string> letters{"X", "Y", "Z"};
    std::vector<std::vector<std::string>> proj_perms;
    for (const auto& perm : views::project_permutations(letters, p3)) {
        proj_perms.push_back(perm);
    }
    assert(proj_perms.size() == 6);
    assert((proj_perms[0] == std::vector<std::string>{"X", "Y", "Z"}));
    assert((proj_perms[5] == std::vector<std::string>{"Z", "Y", "X"}));

    // 3. Integer Partitions View of 4: p(4) = 5
    integer_partitions_view ip4(4);
    assert(ip4.size() == 5);

    std::vector<std::vector<std::size_t>> generated_parts;
    for (auto part : ip4) {
        generated_parts.emplace_back(part.begin(), part.end());
        // Verify partition invariant: sum of elements == 4
        std::size_t s = 0;
        for (std::size_t x : part) s += x;
        assert(s == 4);
    }

    std::vector<std::vector<std::size_t>> expected_parts = {
        {4}, {3, 1}, {2, 2}, {2, 1, 1}, {1, 1, 1, 1}
    };
    assert(generated_parts == expected_parts);

    std::cout << "  -> Passed (Permutations Lexicographical Order & Integer Partitions Invariant)\n";
}

void test_set_partitions_and_equivalence_bridge() {
    std::cout << "[Test] Bridge: Set Partitions RGS & Equivalence Relations Isomorphism...\n";
    using namespace discretex::combinatorics;

    std::size_t n = 4;
    index_domain dom(n);

    // 1. Enumerate all set partitions of {0, 1, 2, 3} via set_partitions_view
    set_partitions_view sp4(n);
    std::size_t total_partitions = 0;
    std::vector<std::size_t> block_counts(n + 1, 0);

    for (auto rgs : sp4) {
        ++total_partitions;
        std::size_t k = block_count(rgs);
        ++block_counts[k];

        // Bridge Test A: Construct relation from RGS partition
        auto rel = relation_from_partition(dom, rgs);

        // Bridge Test B: Verify equivalence relation axioms
        assert(is_equivalence_relation(rel));

        // Bridge Test C: Round-trip: recover canonical RGS from relation
        auto recovered_rgs = equivalence_classes_rgs(rel);
        std::vector<std::size_t> rgs_vec(rgs.begin(), rgs.end());
        assert(recovered_rgs == rgs_vec);

        // Bridge Test D: Explicit blocks round-trip
        auto blocks = equivalence_classes(rel);
        auto rgs_from_blocks = blocks_to_rgs(blocks, n);
        assert(rgs_from_blocks == rgs_vec);

        // Bridge Test E: Construct relation from blocks and verify equivalence with original
        auto rel_from_blocks = relation_from_partition(dom, blocks);
        for (std::size_t u = 0; u < n; ++u) {
            for (std::size_t v = 0; v < n; ++v) {
                assert(rel.contains(u, v) == rel_from_blocks.contains(u, v));
            }
        }
    }

    // 2. Verify total generated equals Bell number B(4) = 15
    assert(total_partitions == bell_number(n));

    // 3. Verify counts per block size match Stirling numbers of the second kind S(4, k)
    assert(block_counts[1] == stirling_second(n, 1)); // 1
    assert(block_counts[2] == stirling_second(n, 2)); // 7
    assert(block_counts[3] == stirling_second(n, 3)); // 6
    assert(block_counts[4] == stirling_second(n, 4)); // 1

    // 4. Test direct k_set_partitions_view(4, 2): must generate exactly S(4, 2) = 7 partitions
    k_set_partitions_view k_sp4_2(4, 2);
    std::size_t k_count = 0;
    for (auto rgs : k_sp4_2) {
        ++k_count;
        assert(block_count(rgs) == 2);
        auto rel = relation_from_partition(dom, rgs);
        assert(is_equivalence_relation(rel));
        assert(quotient_size(rel) == 2);
    }
    assert(k_count == stirling_second(4, 2));

    // 5. Test negative cases for is_equivalence_relation
    dense_relation not_refl(dom); // empty relation: not reflexive
    assert(!is_equivalence_relation(not_refl));

    dense_relation not_sym(dom);
    for (std::size_t i = 0; i < n; ++i) not_sym.add_pair(i, i);
    not_sym.add_pair(0, 1); // (0, 1) without (1, 0)
    assert(!is_equivalence_relation(not_sym));

    dense_relation not_trans(dom);
    for (std::size_t i = 0; i < n; ++i) not_trans.add_pair(i, i);
    not_trans.add_pair(0, 1);
    not_trans.add_pair(1, 0);
    not_trans.add_pair(1, 2);
    not_trans.add_pair(2, 1);
    // Missing (0, 2) and (2, 0)
    assert(!is_equivalence_relation(not_trans));

    std::cout << "  -> Passed (B(4)=15, S(4,k) distributions, RGS round-trip, quotient sets)\n";
}

void test_tarjan_scc_and_condensation() {
    std::cout << "[Test] Graph Theory: Tarjan Strongly Connected Components & Condensation DAG...\n";
    using namespace discretex::algorithms;

    // Graph with 5 vertices:
    // Component 0: {0, 1, 2} (cycle: 0->1, 1->2, 2->0)
    // Edge: 2->3
    // Component 1: {3, 4} (cycle: 3->4, 4->3)
    index_domain dom(5);
    bidirectional_adjacency_graph g(dom);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 0);
    g.add_edge(2, 3);
    g.add_edge(3, 4);
    g.add_edge(4, 3);

    auto scc = tarjan_scc(g);
    assert(scc.component_count == 2);
    assert(scc.component_of[0] == scc.component_of[1]);
    assert(scc.component_of[1] == scc.component_of[2]);
    assert(scc.component_of[3] == scc.component_of[4]);
    assert(scc.component_of[0] != scc.component_of[3]);

    // In topological order of condensation DAG:
    // Component {0, 1, 2} has edge to {3, 4}, so component_of[0] < component_of[3]
    assert(scc.component_of[0] < scc.component_of[3]);

    // Condensation DAG
    auto dag = condensation_dag(g, scc);
    assert(dag.domain_size() == 2);
    assert(dag.edge_count() == 1);
    assert(dag.contains(scc.component_of[0], scc.component_of[3]));
    assert(!dag.contains(scc.component_of[3], scc.component_of[0]));

    // Isolated vertices case: each vertex is its own SCC
    index_domain dom_iso(3);
    bidirectional_adjacency_graph g_iso(dom_iso);
    auto scc_iso = tarjan_scc(g_iso);
    assert(scc_iso.component_count == 3);

    std::cout << "  -> Passed (SCC decomposition, topological component ordering, condensation DAG)\n";
}

void test_two_sat_implication_engine() {
    std::cout << "[Test] Bridge: 2-SAT Satisfiability via Implication Graph & SCC...\n";
    using namespace discretex::logic;

    // 1. Satisfiable Instance:
    // (x0 or x1) and (~x0 or x1) and (x0 or ~x1)
    // Only solution is x0 = true, x1 = true
    formula_2cnf f_sat(2);
    f_sat.add_clause(pos(0), pos(1));
    f_sat.add_clause(neg(0), pos(1));
    f_sat.add_clause(pos(0), neg(1));

    // Inspect implication graph
    auto imp_g = implication_graph(f_sat);
    assert(imp_g.domain_size() == 4); // 2 literals per variable

    auto res_sat = solve_2sat(f_sat);
    assert(res_sat.satisfiable);
    assert(res_sat.assignment.size() == 2);
    assert(res_sat.assignment[0] == true);
    assert(res_sat.assignment[1] == true);

    // Verify assignment against every clause
    for (const auto& cl : f_sat.clauses()) {
        bool left_val = res_sat.assignment[cl.left.var] ^ cl.left.negated;
        bool right_val = res_sat.assignment[cl.right.var] ^ cl.right.negated;
        assert(left_val || right_val);
    }

    // 2. Unsatisfiable Instance:
    // (x0 or x0) and (~x0 or ~x0)
    // Forces x0 to be simultaneously true and false
    formula_2cnf f_unsat(1);
    f_unsat.add_clause(pos(0), pos(0));
    f_unsat.add_clause(neg(0), neg(0));

    auto res_unsat = solve_2sat(f_unsat);
    assert(!res_unsat.satisfiable);
    assert(res_unsat.assignment.empty());

    // 3. 3-Variable Satisfiable Instance:
    // (x0 or x1) and (~x1 or x2) and (~x2 or ~x0)
    formula_2cnf f3(3);
    f3.add_clause(pos(0), pos(1));
    f3.add_clause(neg(1), pos(2));
    f3.add_clause(neg(2), neg(0));

    auto res_f3 = solve_2sat(f3);
    assert(res_f3.satisfiable);
    assert(res_f3.assignment.size() == 3);

    // Cross-bridge check with Propositional Logic AST evaluation from Milestone 4:
    // Formula: (x0 | x1) & (~x1 | x2) & (~x2 | ~x0)
    auto p0 = var(0);
    auto p1 = var(1);
    auto p2 = var(2);
    auto ast_formula = (p0 || p1) && (!p1 || p2) && (!p2 || !p0);

    uint64_t bitmask_model = 0;
    for (std::size_t i = 0; i < 3; ++i) {
        if (res_f3.assignment[i]) {
            bitmask_model |= (1ULL << i);
        }
    }
    assert(evaluate(ast_formula, bitmask_model) == true);

    std::cout << "  -> Passed (Implication graph, Aspvall-Plass-Tarjan satisfiability, model extraction)\n";
}

void test_bipartite_matching_and_koenig() {
    std::cout << "[Test] Graph Theory: Bipartite Matching (Kuhn, Hopcroft-Karp) & Koenig's Theorem...\n";
    using namespace discretex::algorithms;

    // 1. Perfect Matching Case
    // Left: 4 vertices, Right: 4 vertices
    // Edges: (0,0), (0,1), (1,1), (2,2), (3,2), (3,3)
    bipartite_graph g1(4, 4);
    g1.add_edge(0, 0);
    g1.add_edge(0, 1);
    g1.add_edge(1, 1);
    g1.add_edge(2, 2);
    g1.add_edge(3, 2);
    g1.add_edge(3, 3);
    assert(g1.edge_count() == 6);

    auto m_kuhn1 = maximum_bipartite_matching_kuhn(g1);
    auto m_hk1 = maximum_bipartite_matching_hopcroft_karp(g1);
    auto m_canon1 = maximum_bipartite_matching(g1);

    assert(m_kuhn1.matching_size == 4);
    assert(m_hk1.matching_size == 4);
    assert(m_canon1.matching_size == 4);

    // Verify all matched pairs are genuine edges in g1
    for (const auto& [u, v] : m_hk1.edges()) {
        assert(g1.contains(u, v));
    }

    // Verify injective property: no two left vertices share the same right mate
    std::vector<bool> seen_r(4, false);
    for (std::size_t u = 0; u < 4; ++u) {
        if (m_hk1.is_matched_left(u)) {
            std::size_t v = m_hk1.mate_left[u];
            assert(!seen_r[v]);
            seen_r[v] = true;
            assert(m_hk1.mate_right[v] == u);
        }
    }

    // Koenig Theorem verification for g1:
    // Min vertex cover size == Max matching size == 4
    auto cover1 = minimum_vertex_cover(g1, m_hk1);
    assert(cover1.size() == 4);

    // Verify every edge in g1 is covered: u in left_cover OR v in right_cover
    auto is_covered = [&](std::size_t u, std::size_t v, const vertex_cover_result& cov) {
        bool in_l = std::find(cov.left_cover.begin(), cov.left_cover.end(), u) != cov.left_cover.end();
        bool in_r = std::find(cov.right_cover.begin(), cov.right_cover.end(), v) != cov.right_cover.end();
        return in_l || in_r;
    };

    for (std::size_t u = 0; u < g1.left_size(); ++u) {
        for (std::size_t v : g1.right_neighbors(u)) {
            assert(is_covered(u, v, cover1));
        }
    }

    // 2. Bottleneck Case (Hall condition violation for full matching)
    // Left: 3 vertices, Right: 3 vertices
    // 0 -> {0, 1}, 1 -> {0, 1}, 2 -> {0, 1}. (Only 2 right vertices available for 3 left vertices)
    bipartite_graph g2(3, 3);
    g2.add_edge(0, 0);
    g2.add_edge(0, 1);
    g2.add_edge(1, 0);
    g2.add_edge(1, 1);
    g2.add_edge(2, 0);
    g2.add_edge(2, 1);

    auto m_kuhn2 = maximum_bipartite_matching_kuhn(g2);
    auto m_hk2 = maximum_bipartite_matching_hopcroft_karp(g2);

    assert(m_kuhn2.matching_size == 2);
    assert(m_hk2.matching_size == 2);

    // Koenig theorem for g2: Cover size must be exactly 2
    auto cover2 = minimum_vertex_cover(g2, m_hk2);
    assert(cover2.size() == 2);
    for (std::size_t u = 0; u < g2.left_size(); ++u) {
        for (std::size_t v : g2.right_neighbors(u)) {
            assert(is_covered(u, v, cover2));
        }
    }

    // 3. Empty and Single Edge graphs
    bipartite_graph g_empty(2, 2);
    assert(maximum_bipartite_matching(g_empty).matching_size == 0);

    bipartite_graph g_single(2, 2);
    g_single.add_edge(0, 1);
    assert(maximum_bipartite_matching(g_single).matching_size == 1);

    std::cout << "  -> Passed (Kuhn == Hopcroft-Karp, Koenig |M|==|C|, injectivity certification)\n";
}

void test_dsu_and_minimum_spanning_tree() {
    std::cout << "[Test] Graph Theory: DSU, Weighted Graphs, MST (Kruskal, Prim) & Invariants...\n";
    using namespace discretex;
    using namespace discretex::algorithms;

    // 1. Test DSU functionality
    disjoint_set dsu(6);
    assert(dsu.size() == 6);
    assert(dsu.component_count() == 6);
    for (std::size_t i = 0; i < 6; ++i) {
        assert(dsu.component_size(i) == 1);
        assert(dsu.connected(i, i));
    }

    assert(dsu.unite(0, 1) == true);
    assert(dsu.unite(0, 1) == false); // already united
    assert(dsu.unite(2, 3) == true);
    assert(dsu.unite(4, 5) == true);
    assert(dsu.component_count() == 3);
    assert(dsu.component_size(0) == 2);
    assert(dsu.component_size(1) == 2);
    assert(dsu.connected(0, 1));
    assert(!dsu.connected(0, 2));

    assert(dsu.unite(1, 3) == true);
    assert(dsu.component_count() == 2);
    assert(dsu.component_size(0) == 4);
    assert(dsu.connected(0, 3));
    assert(dsu.connected(1, 2));

    auto comps = dsu.components();
    assert(comps.size() == 2);

    // Bridge: DSU to RGS and Equivalence Relation
    auto rgs = dsu.to_rgs();
    assert(rgs.size() == 6);
    // Elements in same set must have same RGS block
    assert(rgs[0] == rgs[1] && rgs[1] == rgs[2] && rgs[2] == rgs[3]);
    assert(rgs[4] == rgs[5]);
    assert(rgs[0] != rgs[4]);

    // Check equivalence relation generated from this RGS
    auto rel = relation_from_partition(index_domain(6), rgs);
    assert(is_equivalence_relation(rel));
    assert(rel.contains(0, 3) && rel.contains(3, 0));
    assert(!rel.contains(0, 4));

    // 2. Test Weighted Undirected Graph & MST on Connected Graph
    // 5-vertex graph:
    // Vertices: 0, 1, 2, 3, 4
    // Edges:
    // (0, 1, 3), (0, 3, 7), (0, 4, 8)
    // (1, 2, 1), (1, 3, 4)
    // (2, 3, 2)
    // (3, 4, 3)
    weighted_undirected_graph<int> g(5);
    g.add_edge(0, 1, 3);
    g.add_edge(0, 3, 7);
    g.add_edge(0, 4, 8);
    g.add_edge(1, 2, 1);
    g.add_edge(1, 3, 4);
    g.add_edge(2, 3, 2);
    g.add_edge(3, 4, 3);

    assert(g.vertex_count() == 5);
    assert(g.edge_count() == 7);
    assert(g.has_edge(1, 2));
    assert(!g.has_edge(0, 2));
    assert(g.edge_weight(1, 2) == 1);
    assert(g.edge_weight(0, 2) == std::nullopt);

    auto mst_k = minimum_spanning_tree_kruskal(g);
    auto mst_p = minimum_spanning_tree_prim(g);
    auto mst_c = minimum_spanning_tree(g);

    // MST edges: (1, 2, 1), (2, 3, 2), (0, 1, 3), (3, 4, 3)
    // Total weight = 1 + 2 + 3 + 3 = 9.
    assert(mst_k.total_weight == 9);
    assert(mst_p.total_weight == 9);
    assert(mst_c.total_weight == 9);
    assert(mst_k.edges.size() == 4);
    assert(mst_p.edges.size() == 4);
    assert(mst_k.is_connected && mst_p.is_connected);
    assert(mst_k.component_count == 1);
    assert(mst_p.component_count == 1);

    // Verify invariants
    assert(is_valid_spanning_forest(g, mst_k));
    assert(is_valid_spanning_forest(g, mst_p));

    // 3. Brute-force verification on small graph:
    // Compare Kruskal and Prim against exact global minimum over all 4-edge subsets
    // that form spanning trees using views::k_subsets
    int brute_force_min_weight = 999999;
    for (auto subset : views::k_subsets(g.edge_count(), g.vertex_count() - 1)) {
        disjoint_set check_dsu(g.vertex_count());
        bool has_cycle = false;
        int current_weight = 0;
        for (std::size_t edge_idx : subset) {
            const auto& e = g.edges()[edge_idx];
            current_weight += e.weight;
            if (!check_dsu.unite(e.u, e.v)) {
                has_cycle = true;
                break;
            }
        }
        if (!has_cycle && check_dsu.component_count() == 1) {
            if (current_weight < brute_force_min_weight) {
                brute_force_min_weight = current_weight;
            }
        }
    }
    assert(brute_force_min_weight == 9);
    assert(mst_k.total_weight == brute_force_min_weight);

    // 4. Test Disconnected Graph (Minimum Spanning Forest)
    // Component 1: vertices 0, 1, 2. Edges: (0, 1, 4), (1, 2, 2), (0, 2, 5) -> min weight = 2 + 4 = 6
    // Component 2: vertices 3, 4. Edges: (3, 4, 7) -> min weight = 7
    // Component 3: vertex 5 (isolated) -> min weight = 0
    weighted_undirected_graph<int> g_disc(6);
    g_disc.add_edge(0, 1, 4);
    g_disc.add_edge(1, 2, 2);
    g_disc.add_edge(0, 2, 5);
    g_disc.add_edge(3, 4, 7);

    assert(connected_component_count(g_disc) == 3);
    auto disc_comps = connected_components(g_disc);
    assert(disc_comps.size() == 3);

    auto msf_k = minimum_spanning_tree_kruskal(g_disc);
    auto msf_p = minimum_spanning_tree_prim(g_disc);

    assert(!msf_k.is_connected);
    assert(!msf_p.is_connected);
    assert(msf_k.component_count == 3);
    assert(msf_p.component_count == 3);
    // Number of edges in forest = |V| - c = 6 - 3 = 3
    assert(msf_k.edges.size() == 3);
    assert(msf_p.edges.size() == 3);
    assert(msf_k.total_weight == 13); // 6 + 7
    assert(msf_p.total_weight == 13);

    assert(is_valid_spanning_forest(g_disc, msf_k));
    assert(is_valid_spanning_forest(g_disc, msf_p));

    std::cout << "  -> Passed (DSU, Kruskal == Prim, brute-force optimality, MSF invariant |V|-c)\n";
}

void test_network_flow_and_max_flow_min_cut() {
    std::cout << "[Test] Graph Theory: Network Flow (Edmonds-Karp, Dinic) & Max-Flow Min-Cut Bridge...\n";
    using namespace discretex;
    using namespace discretex::algorithms;

    // 1. Classic Network Flow
    // Vertices: 0 (source), 1, 2, 3, 4, 5 (sink)
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

    assert(net.vertex_count() == 6);
    assert(net.edge_count() == 10);

    auto res_ek = max_flow_edmonds_karp(net, 0, 5);
    auto res_dinic = max_flow_dinic(net, 0, 5);
    auto res_canon = max_flow(net, 0, 5);

    // Textbook solution: max flow = 23
    assert(res_ek.max_flow == 23);
    assert(res_dinic.max_flow == 23);
    assert(res_canon.max_flow == 23);

    // Verify flow conservation & capacity limits
    assert(verify_flow_conservation(net, res_ek, 0, 5));
    assert(verify_flow_conservation(net, res_dinic, 0, 5));

    // Verify Max-Flow Min-Cut theorem: max flow == cut capacity
    assert(res_ek.source_side_min_cut[0] == true);
    assert(res_ek.source_side_min_cut[5] == false);
    assert(res_dinic.source_side_min_cut[0] == true);
    assert(res_dinic.source_side_min_cut[5] == false);

    int cut_cap_ek = compute_cut_capacity(net, res_ek.source_side_min_cut);
    int cut_cap_dinic = compute_cut_capacity(net, res_dinic.source_side_min_cut);
    assert(cut_cap_ek == 23);
    assert(cut_cap_dinic == 23);

    // 2. Parallel Edges and Disconnected Networks
    flow_network<int> net_parallel(3);
    net_parallel.add_edge(0, 1, 5);
    net_parallel.add_edge(0, 1, 10); // parallel edge
    net_parallel.add_edge(1, 2, 12);
    auto res_parallel = max_flow_dinic(net_parallel, 0, 2);
    assert(res_parallel.max_flow == 12); // bottleneck is 12
    assert(compute_cut_capacity(net_parallel, res_parallel.source_side_min_cut) == 12);

    flow_network<int> net_disc(4);
    net_disc.add_edge(0, 1, 10);
    net_disc.add_edge(2, 3, 10);
    auto res_disc = max_flow_dinic(net_disc, 0, 3);
    assert(res_disc.max_flow == 0);

    // 3. Cross-module Bridge: Bipartite Matching via Max Flow Reduction
    // Create the same bipartite graph tested in test_bipartite_matching_and_koenig():
    // Left: 4 vertices, Right: 4 vertices
    // Edges: (0,0), (0,1), (1,1), (2,2), (3,2), (3,3)
    bipartite_graph bg(4, 4);
    bg.add_edge(0, 0);
    bg.add_edge(0, 1);
    bg.add_edge(1, 1);
    bg.add_edge(2, 2);
    bg.add_edge(3, 2);
    bg.add_edge(3, 3);

    auto m_hk = maximum_bipartite_matching_hopcroft_karp(bg);
    assert(m_hk.matching_size == 4);

    // Reduction to Unit Flow Network:
    // S = 0
    // Left: 1, 2, 3, 4 (u -> 1 + u)
    // Right: 5, 6, 7, 8 (v -> 5 + v)
    // T = 9
    flow_network<int> flow_match_net(10);
    std::size_t S = 0, T = 9;
    for (std::size_t u = 0; u < 4; ++u) {
        flow_match_net.add_edge(S, 1 + u, 1);
    }
    for (std::size_t u = 0; u < 4; ++u) {
        for (std::size_t v : bg.right_neighbors(u)) {
            flow_match_net.add_edge(1 + u, 5 + v, 1);
        }
    }
    for (std::size_t v = 0; v < 4; ++v) {
        flow_match_net.add_edge(5 + v, T, 1);
    }

    auto flow_match_res = max_flow_dinic(flow_match_net, S, T);
    // 1. Max flow must equal maximum matching size!
    assert(flow_match_res.max_flow == static_cast<int>(m_hk.matching_size));

    // 2. The min-cut induces Koenig's minimum vertex cover:
    // Left vertices on sink side (not in source_side_min_cut)
    // Right vertices on source side (in source_side_min_cut)
    std::vector<std::size_t> flow_cover_l;
    std::vector<std::size_t> flow_cover_r;
    for (std::size_t u = 0; u < 4; ++u) {
        if (!flow_match_res.source_side_min_cut[1 + u]) {
            flow_cover_l.push_back(u);
        }
    }
    for (std::size_t v = 0; v < 4; ++v) {
        if (flow_match_res.source_side_min_cut[5 + v]) {
            flow_cover_r.push_back(v);
        }
    }

    // Min cover cardinality == Max matching size
    assert(flow_cover_l.size() + flow_cover_r.size() == m_hk.matching_size);

    // Assert that every bipartite edge is covered by (flow_cover_l union flow_cover_r)
    auto is_covered = [&](std::size_t u, std::size_t v) {
        bool in_l = std::find(flow_cover_l.begin(), flow_cover_l.end(), u) != flow_cover_l.end();
        bool in_r = std::find(flow_cover_r.begin(), flow_cover_r.end(), v) != flow_cover_r.end();
        return in_l || in_r;
    };
    for (std::size_t u = 0; u < bg.left_size(); ++u) {
        for (std::size_t v : bg.right_neighbors(u)) {
            assert(is_covered(u, v));
        }
    }

    std::cout << "  -> Passed (Edmonds-Karp == Dinic, Max-Flow Min-Cut duality, Bipartite Matching reduction)\n";
}

void test_shortest_paths_suite() {
    std::cout << "[Test] Graph Theory: Shortest Paths (DAG, Dijkstra, Bellman-Ford, Floyd-Warshall)...\n";
    using namespace discretex;
    using namespace discretex::algorithms;

    // 1. Test DAG Shortest Paths (supports negative edge weights)
    weighted_directed_graph<int> dag(5);
    dag.add_edge(0, 1, 3);
    dag.add_edge(0, 2, 2);
    dag.add_edge(1, 3, 4);
    dag.add_edge(1, 4, 1);
    dag.add_edge(2, 1, -1); // negative edge
    dag.add_edge(2, 3, 5);
    dag.add_edge(3, 4, 2);

    auto dag_res = dag_shortest_paths<int>(dag, 0);
    assert(dag_res.distance[0] == 0);
    assert(dag_res.distance[1] == 1); // 0 -> 2 -> 1
    assert(dag_res.distance[2] == 2); // 0 -> 2
    assert(dag_res.distance[3] == 5); // 0 -> 2 -> 1 -> 3
    assert(dag_res.distance[4] == 2); // 0 -> 2 -> 1 -> 4

    // Cross-check with Bellman-Ford on DAG
    auto bf_dag = bellman_ford_shortest_paths<int>(dag, 0);
    assert(!bf_dag.has_negative_cycle);
    for (std::size_t i = 0; i < 5; ++i) {
        assert(dag_res.distance[i] == bf_dag.distance[i]);
    }

    // Lazy path reconstruction and path integrity
    auto path_to_4 = reconstruct_path(dag_res, 4);
    assert(path_to_4.has_value());
    std::vector<std::size_t> expected_p4 = {0, 2, 1, 4};
    assert(*path_to_4 == expected_p4);
    assert(verify_path_integrity(dag, *path_to_4, 2));

    // DAG cycle rejection
    weighted_directed_graph<int> cyclic(3);
    cyclic.add_edge(0, 1, 1);
    cyclic.add_edge(1, 2, 1);
    cyclic.add_edge(2, 0, 1);
    bool threw_cycle = false;
    try {
        dag_shortest_paths<int>(cyclic, 0);
    } catch (const std::invalid_argument&) {
        threw_cycle = true;
    }
    assert(threw_cycle);

    // 2. Test Dijkstra Algorithm
    weighted_directed_graph<int> g_dijkstra(6);
    g_dijkstra.add_edge(0, 1, 4);
    g_dijkstra.add_edge(0, 2, 2);
    g_dijkstra.add_edge(1, 2, 1);
    g_dijkstra.add_edge(1, 3, 5);
    g_dijkstra.add_edge(2, 3, 8);
    g_dijkstra.add_edge(2, 4, 10);
    g_dijkstra.add_edge(3, 4, 2);
    // vertex 5 is unreachable

    auto dijk_res = dijkstra_shortest_paths<int>(g_dijkstra, 0);
    assert(dijk_res.distance[0] == 0);
    assert(dijk_res.distance[1] == 4);
    assert(dijk_res.distance[2] == 2);
    assert(dijk_res.distance[3] == 9);  // 0 -> 1 -> 3
    assert(dijk_res.distance[4] == 11); // 0 -> 1 -> 3 -> 4
    assert(!dijk_res.is_reachable(5));
    assert(dijk_res.distance[5] == std::nullopt);

    auto p_dijk = reconstruct_path(dijk_res, 4);
    assert(p_dijk.has_value());
    assert(verify_path_integrity(g_dijkstra, *p_dijk, 11));
    assert(!reconstruct_path(dijk_res, 5).has_value());

    // Cross-check Dijkstra vs Bellman-Ford on non-negative graph
    auto bf_dijk = bellman_ford_shortest_paths<int>(g_dijkstra, 0);
    assert(!bf_dijk.has_negative_cycle);
    for (std::size_t i = 0; i < 6; ++i) {
        assert(dijk_res.distance[i] == bf_dijk.distance[i]);
    }

    // 3. Test Bellman-Ford with Negative Weights and Cycle Detection
    weighted_directed_graph<int> g_neg(4);
    g_neg.add_edge(0, 1, 4);
    g_neg.add_edge(0, 2, 5);
    g_neg.add_edge(1, 2, -2);
    g_neg.add_edge(2, 3, 3);

    auto bf_res = bellman_ford_shortest_paths<int>(g_neg, 0);
    assert(!bf_res.has_negative_cycle);
    assert(bf_res.distance[0] == 0);
    assert(bf_res.distance[1] == 4);
    assert(bf_res.distance[2] == 2);
    assert(bf_res.distance[3] == 5);
    auto p_bf = reconstruct_path(bf_res, 3);
    assert(p_bf.has_value());
    assert(verify_path_integrity(g_neg, *p_bf, 5));

    // Negative cycle detection in Bellman-Ford
    weighted_directed_graph<int> g_cycle(4);
    g_cycle.add_edge(0, 1, 1);
    g_cycle.add_edge(1, 2, 2);
    g_cycle.add_edge(2, 3, 3);
    g_cycle.add_edge(3, 1, -10); // cycle 1 -> 2 -> 3 -> 1 has sum 2 + 3 - 10 = -5 < 0

    auto bf_cycle = bellman_ford_shortest_paths<int>(g_cycle, 0);
    assert(bf_cycle.has_negative_cycle == true);
    assert(!reconstruct_path(bf_cycle, 3).has_value());

    // 4. Test Floyd-Warshall All-Pairs Algorithm
    weighted_directed_graph<int> g_fw(4);
    g_fw.add_edge(0, 1, 1);
    g_fw.add_edge(1, 2, 2);
    g_fw.add_edge(2, 3, 3);
    g_fw.add_edge(3, 0, 4);

    auto fw_res = floyd_warshall_all_pairs<int>(g_fw);
    assert(!fw_res.has_negative_cycle);

    // Cross-check: Each row in Floyd-Warshall matches Dijkstra single source
    for (std::size_t s = 0; s < 4; ++s) {
        auto d_row = dijkstra_shortest_paths<int>(g_fw, s);
        for (std::size_t t = 0; t < 4; ++t) {
            assert(fw_res.distance[s][t] == d_row.distance[t]);
        }
    }

    auto fw_p03 = reconstruct_path(fw_res, 0, 3);
    assert(fw_p03.has_value());
    std::vector<std::size_t> expected_fw = {0, 1, 2, 3};
    assert(*fw_p03 == expected_fw);
    assert(verify_path_integrity(g_fw, *fw_p03, 6));

    // Floyd-Warshall negative cycle detection
    weighted_directed_graph<int> g_fw_neg(3);
    g_fw_neg.add_edge(0, 1, 1);
    g_fw_neg.add_edge(1, 2, 2);
    g_fw_neg.add_edge(2, 0, -5); // cycle sum -2 < 0
    auto fw_neg_res = floyd_warshall_all_pairs<int>(g_fw_neg);
    assert(fw_neg_res.has_negative_cycle == true);

    // 5. Undirected Graph Compatibility & Distance Symmetry
    weighted_undirected_graph<int> g_undir(4);
    g_undir.add_edge(0, 1, 2);
    g_undir.add_edge(1, 2, 3);
    g_undir.add_edge(2, 3, 4);
    g_undir.add_edge(0, 3, 15);

    auto dijk_undir = dijkstra_shortest_paths<int>(g_undir, 0);
    assert(dijk_undir.distance[3] == 9); // 0-1-2-3 (2+3+4) instead of direct 15
    auto fw_undir = floyd_warshall_all_pairs<int>(g_undir);
    // Symmetry in undirected distance matrix: dist[u][v] == dist[v][u]
    for (std::size_t u = 0; u < 4; ++u) {
        for (std::size_t v = 0; v < 4; ++v) {
            assert(fw_undir.distance[u][v] == fw_undir.distance[v][u]);
        }
    }

    std::cout << "  -> Passed (DAG, Dijkstra, Bellman-Ford, Floyd-Warshall, cross-validations, cycle detection)\n";
}

void test_algebra_subsystem() {
    std::cout << "[Test] Abstract Algebra: Operation Tables, Laws, Groups, Morphisms & Boolean Algebras...\n";
    using namespace discretex;
    using namespace discretex::algebra;

    // 1. Operation Table & Laws Verification
    // Non-associative operation: Subtraction modulo 5: (a - b) mod 5
    auto sub_table = operation_table<index_domain>::from_callable(
        index_domain(5),
        [](std::size_t a, std::size_t b) {
            return (a + 5 - b) % 5;
        });
    assert(!is_associative(sub_table)); // (1 - 2) - 3 = 4 - 3 = 1 != 1 - (2 - 3) = 1 - 4 = 2

    // Associative & Commutative: Addition modulo 6
    auto add_table = operation_table<index_domain>::from_callable(
        index_domain(6),
        [](std::size_t a, std::size_t b) {
            return (a + b) % 6;
        });
    assert(is_associative(add_table));
    assert(is_commutative(add_table));
    auto id_add = find_identity(add_table);
    assert(id_add.has_value() && *id_add == 0);
    assert(has_inverses(add_table, 0));
    auto inv_add = inverse_table(add_table, 0);
    assert(inv_add.has_value());
    assert((*inv_add)[0] == 0 && (*inv_add)[1] == 5 && (*inv_add)[2] == 4);

    // 2. Finite Monoids
    // Multiplication modulo 6: (Z/6Z, * mod 6)
    // Associative with identity 1, but not a group because 2, 3, 4, 0 have no inverses.
    auto mul6 = finite_monoid<index_domain>::from_callable(
        index_domain(6),
        [](std::size_t a, std::size_t b) {
            return (a * b) % 6;
        });
    assert(mul6.identity() == 1);
    assert(mul6.op(2, 3) == 0);
    assert(mul6.is_commutative());
    assert(!has_inverses(mul6.operation(), 1));

    // 3. Finite Groups: Cyclic Group Z_6 and Element Orders
    auto z6 = cyclic_group(6);
    assert(z6.size() == 6);
    assert(z6.order() == 6);
    assert(z6.identity() == 0);
    assert(z6.is_abelian());
    assert(z6.element_order(0) == 1);
    assert(z6.element_order(1) == 6);
    assert(z6.element_order(2) == 3);
    assert(z6.element_order(3) == 2);
    assert(z6.element_order(4) == 3);
    assert(z6.element_order(5) == 6);

    // Subgroup verification:
    // H = {0, 2, 4} is a subgroup of Z_6 (order 3, Lagrange theorem holds: 3 | 6)
    assert(z6.is_subgroup({0, 2, 4}));
    // H = {0, 3} is a subgroup of Z_6 (order 2, Lagrange theorem holds: 2 | 6)
    assert(z6.is_subgroup({0, 3}));
    // H = {0, 1} is not a subgroup (1 + 1 = 2 not in H)
    assert(!z6.is_subgroup({0, 1}));

    // Multiplicative Unit Group (Z/8Z)*:
    // Elements coprime to 8: {1, 3, 5, 7}, size phi(8) = 4.
    auto u8 = unit_group_mod_n(8);
    assert(u8.order() == 4);
    assert(u8.is_abelian());
    // In (Z/8Z)*, every element squared is 1: 1^2=1, 3^2=9=1, 5^2=25=1, 7^2=49=1!
    for (std::size_t i = 0; i < 4; ++i) {
        if (i == u8.identity()) {
            assert(u8.element_order(i) == 1);
        } else {
            assert(u8.element_order(i) == 2);
        }
    }

    // 4. Morphisms: Homomorphisms and Isomorphisms
    // A. Natural projection homomorphism pi: Z_6 -> Z_3: pi(x) = x mod 3
    auto z3 = cyclic_group(3);
    auto pi = mod_reduction_homomorphism(6, 3);
    assert(is_homomorphism(pi, z6, z3));
    assert(is_surjective(pi, 3));
    assert(!is_injective(pi));
    assert(!is_isomorphism(pi, z6, z3));

    // First Isomorphism Theorem check:
    // Ker(pi) = {x in Z_6 | pi(x) == 0} = {0, 3}
    auto ker = kernel(pi, z3.identity());
    std::vector<std::size_t> expected_ker = {0, 3};
    assert(ker == expected_ker);
    assert(z6.is_subgroup(ker)); // Kernel is a subgroup!

    // Image(pi) = {0, 1, 2}
    auto im = image(pi);
    std::vector<std::size_t> expected_im = {0, 1, 2};
    assert(im == expected_im);

    // B. Group Isomorphism: Klein 4-group V_4 isomorphic to (Z/8Z)*
    auto v4 = klein_four_group();
    assert(v4.order() == 4);
    // Both V_4 and U_8 are elementary abelian groups of order 4 (Z_2 x Z_2).
    // The units {1, 3, 5, 7} map to indices: 1->0 (id), 3->1, 5->2, 7->3.
    // 3 * 5 = 15 = 7 mod 8 (index 1 * 2 = 3).
    // In V4: 1 ^ 2 = 3. Perfectly matches!
    std::vector<std::size_t> v4_to_u8 = {0, 1, 2, 3};
    assert(is_isomorphism(v4_to_u8, v4, u8));

    // 5. Finite Boolean Algebra
    // Power set Boolean algebra on 3 elements: B_3 = P({0, 1, 2}) of size 8
    auto b3 = power_set_boolean_algebra(3);
    assert(b3.size() == 8);
    assert(b3.bottom() == 0);
    assert(b3.top() == 7);

    // Verify Boolean algebraic properties
    for (std::size_t a = 0; a < 8; ++a) {
        // Idempotence
        assert(b3.join(a, a) == a);
        assert(b3.meet(a, a) == a);
        // Identity
        assert(b3.join(a, b3.bottom()) == a);
        assert(b3.meet(a, b3.top()) == a);
        // Complements
        assert(b3.join(a, b3.complement(a)) == b3.top());
        assert(b3.meet(a, b3.complement(a)) == b3.bottom());
        // Double complement (involution)
        assert(b3.complement(b3.complement(a)) == a);

        for (std::size_t b = 0; b < 8; ++b) {
            // De Morgan's laws: ~(a \/ b) == ~a /\ ~b
            std::size_t not_a_or_b = b3.complement(b3.join(a, b));
            std::size_t not_a_and_not_b = b3.meet(b3.complement(a), b3.complement(b));
            assert(not_a_or_b == not_a_and_not_b);

            // ~(a /\ b) == ~a \/ ~b
            std::size_t not_a_and_b = b3.complement(b3.meet(a, b));
            std::size_t not_a_or_not_b = b3.join(b3.complement(a), b3.complement(b));
            assert(not_a_and_b == not_a_or_not_b);
        }
    }

    std::cout << "  -> Passed (Tables, Laws, Monoid, Z_6 Subgroups, (Z/8Z)* =~ V_4, Boolean Algebra De Morgan)\n";
}

void test_quotient_groups_and_isomorphism_theorem() {
    std::cout << "[Test] Abstract Algebra: Normal Subgroups, Cosets, Quotient Groups & First Isomorphism Theorem...\n";
    using namespace discretex;
    using namespace discretex::algebra;

    // 1. Non-abelian group test: Symmetric Group S_3 (order 6)
    // Permutations of {0, 1, 2}
    std::vector<std::vector<std::size_t>> perms = {
        {0, 1, 2}, // 0: identity (1)
        {1, 2, 0}, // 1: (0 1 2) rotation
        {2, 0, 1}, // 2: (0 2 1) rotation
        {0, 2, 1}, // 3: (1 2) transposition
        {2, 1, 0}, // 4: (0 2) transposition
        {1, 0, 2}  // 5: (0 1) transposition
    };
    auto s3_op = operation_table<index_domain>::from_callable(
        index_domain(6),
        [&](std::size_t i, std::size_t j) {
            std::vector<std::size_t> comp = {
                perms[i][perms[j][0]],
                perms[i][perms[j][1]],
                perms[i][perms[j][2]]
            };
            for (std::size_t k = 0; k < 6; ++k) {
                if (perms[k] == comp) return k;
            }
            return 6UL;
        });

    finite_group<index_domain> s3(index_domain(6), std::move(s3_op));
    assert(!s3.is_abelian());
    assert(s3.order() == 6);

    // Alternating group A_3 = {0, 1, 2} has index 2: must be NORMAL!
    std::vector<std::size_t> A3 = {0, 1, 2};
    assert(s3.is_subgroup(A3));
    assert(is_normal_subgroup(s3, A3));

    // Subgroup H = {0, 3} (generated by transposition (1 2)) has index 3: NOT NORMAL!
    // Conjugation: (0 1) * (1 2) * (0 1)^-1 = (0 2) (index 4) not in H!
    std::vector<std::size_t> H_trans = {0, 3};
    assert(s3.is_subgroup(H_trans));
    assert(!is_normal_subgroup(s3, H_trans));

    // Construct quotient S_3 / A_3: must have order 6 / 3 = 2 (isomorphic to Z_2)
    auto q_s3 = quotient_group(s3, A3);
    assert(q_s3.quotient.size() == 2);
    assert(q_s3.cosets.size() == 2);
    assert(q_s3.cosets[0] == A3);
    std::vector<std::size_t> expected_coset1 = {3, 4, 5};
    assert(q_s3.cosets[1] == expected_coset1);

    // Attempting quotient by non-normal subgroup H_trans must throw std::invalid_argument
    bool threw_non_normal = false;
    try {
        quotient_group(s3, H_trans);
    } catch (const std::invalid_argument&) {
        threw_non_normal = true;
    }
    assert(threw_non_normal);

    // 2. Cosets and Quotient in Abelian Group Z_6 / {0, 3}
    auto z6 = cyclic_group(6);
    std::vector<std::size_t> H_z6 = {0, 3};
    assert(is_normal_subgroup(z6, H_z6)); // all subgroups in abelian group are normal

    auto [cosets_z6, proj_z6] = coset_partition(z6, H_z6);
    assert(cosets_z6.size() == 3);
    assert(proj_z6[0] == 0 && proj_z6[3] == 0);
    assert(proj_z6[1] == 1 && proj_z6[4] == 1);
    assert(proj_z6[2] == 2 && proj_z6[5] == 2);

    auto q_z6 = quotient_group(z6, H_z6);
    assert(q_z6.quotient.size() == 3);
    assert(q_z6.quotient.is_abelian());
    assert(q_z6.quotient.element_order(1) == 3); // isomorphic to Z_3

    // 3. First Isomorphism Theorem Certification: G / ker(f) =~ im(f)
    // Instance A: pi: Z_6 -> Z_3, pi(x) = x mod 3
    auto z3 = cyclic_group(3);
    auto pi = mod_reduction_homomorphism(6, 3);
    auto cert_z6 = certify_first_isomorphism_theorem(z6, z3, pi);
    assert(cert_z6.is_valid_isomorphism);
    assert(cert_z6.kernel == H_z6);
    std::vector<std::size_t> expected_im_z3 = {0, 1, 2};
    assert(cert_z6.image == expected_im_z3);
    assert(cert_z6.quotient_res.quotient.size() == 3);
    assert(cert_z6.image_group.size() == 3);

    // Instance B: psi: Z_12 -> Z_4, psi(x) = x mod 4
    auto z12 = cyclic_group(12);
    auto z4 = cyclic_group(4);
    auto psi = mod_reduction_homomorphism(12, 4);
    auto cert_z12 = certify_first_isomorphism_theorem(z12, z4, psi);
    assert(cert_z12.is_valid_isomorphism);
    std::vector<std::size_t> expected_ker_z12 = {0, 4, 8};
    assert(cert_z12.kernel == expected_ker_z12);
    assert(cert_z12.quotient_res.quotient.size() == 4);
    assert(cert_z12.image_group.size() == 4);

    std::cout << "  -> Passed (Normal subgroups, non-normal rejection, S_3/A_3, First Isomorphism Theorem certified)\n";
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
    test_counting_functions();
    test_permutations_and_integer_partitions();
    test_set_partitions_and_equivalence_bridge();
    test_tarjan_scc_and_condensation();
    test_two_sat_implication_engine();
    test_bipartite_matching_and_koenig();
    test_dsu_and_minimum_spanning_tree();
    test_network_flow_and_max_flow_min_cut();
    test_shortest_paths_suite();
    test_algebra_subsystem();
    test_quotient_groups_and_isomorphism_theorem();

    std::cout << "\nALL TESTS PASSED SUCCESSFULLY (100%)\n";
    return 0;
}





