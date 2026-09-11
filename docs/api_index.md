# DiscreteX API Quick Reference Index

This index provides a concise, navigable lookup for all 34 header files in DiscreteX.

---

## 1. Core Foundations & Concepts

| Header | Major Types / Concepts | Key Operations | One-Line Summary |
| :--- | :--- | :--- | :--- |
| [`core/domain.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/core/domain.hpp) | `index_domain`, `mapped_domain<T>` | `domain_size()`, `index_of(x)`, `element(i)` | Dense contiguous index spaces and semantic bidirectional value-to-index mapping adapters. |
| [`core/dsu.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/core/dsu.hpp) | `disjoint_set` (alias `dsu`) | `find(u)`, `unite(u, v)`, `connected(u, v)`, `to_rgs()` | Disjoint Set Union with path compression and union-by-size in amortized $O(\alpha(n))$ time. |
| [`core/bit_row_fiber_view.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/core/bit_row_fiber_view.hpp) | `bit_row_fiber_view` | Forward iterators over active bits | Hardware-accelerated bit-scan iteration over 64-bit packed words using `std::countr_zero`. |
| [`concepts/domain.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/concepts/domain.hpp) | `FiniteDomain`, `IndexableDomain<T>` | `domain_size(d)`, `index_of(d, x)` | C++20 concept constraints for finite and indexable sets. |
| [`concepts/relation.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/concepts/relation.hpp) | `Relation`, `ForwardRelation`, `BidirectionalRelation` | `has_edge(u, v)`, `out_neighbors(u)`, `in_neighbors(u)` | C++20 concepts governing mathematical binary relations and fiber accessors. |
| [`concepts/graph.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/concepts/graph.hpp) | `ForwardGraph`, `BidirectionalGraph` | `vertex_count()`, `edge_count()` | C++20 concepts defining graph structural contracts over domains. |

---

## 2. Relations and Quotients

| Header | Major Types | Key Functions | One-Line Summary |
| :--- | :--- | :--- | :--- |
| [`relation/dense_relation.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/relation/dense_relation.hpp) | `dense_relation` | `add_edge(u, v)`, `transitive_closure()`, `out_fiber(u)` | Packed 64-bit bit-matrix binary relation with in-place $O(n^3/64)$ Warshall transitive closure. |
| [`relation/equivalence.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/relation/equivalence.hpp) | Partition vectors | `is_equivalence_relation(r)`, `equivalence_classes(r)`, `relation_from_partition(p)` | Equivalence axiom verification, canonical quotient extraction, and partition-to-relation bridge. |
| [`relation/transpose_view.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/relation/transpose_view.hpp) | `transpose_view<Rel>` | `views::transpose(r)` | Zero-copy converse relation view $R^{-1}$ swapping in- and out-neighbor accessors. |

---

## 3. Graph Structures

| Header | Major Types | Key Functions | One-Line Summary |
| :--- | :--- | :--- | :--- |
| [`graph/sparse_graph.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/graph/sparse_graph.hpp) | `forward_adjacency_graph`, `bidirectional_adjacency_graph` | `from_edges(dom, edges)`, `out_neighbors(u)` | Adjacency list graphs with sorted neighbor vectors and $O(\log \text{deg})$ edge lookups. |
| [`graph/weighted_graph.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/graph/weighted_graph.hpp) | `weighted_directed_graph<W>`, `weighted_undirected_graph<W>` | `add_edge(u, v, w)`, `edge_weight(u, v)` | Flat-edge and adjacency-list models for arbitrary ordered edge weights. |
| [`graph/bipartite_graph.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/graph/bipartite_graph.hpp) | `bipartite_graph<DomL, DomR>` | `add_edge(l, r)`, `left_neighbors(l)` | Explicit partitioned graph $G = (L \cup R, E)$ with zero-overhead side-local vertex indices. |
| [`graph/flow_network.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/graph/flow_network.hpp) | `flow_network<Cap>`, `residual_edge<Cap>` | `add_edge(u, v, cap)`, `residual_edges(u)` | Semantic flow networks with linked forward and reverse residual edge pairs. |

---

## 4. Graph Algorithms & Traversal

| Header | Key Algorithms | Return Types | One-Line Summary |
| :--- | :--- | :--- | :--- |
| [`algorithms/bfs.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algorithms/bfs.hpp) | `bfs(graph, src)` | `bfs_result` | Generic breadth-first search operating uniformly over dense relations and sparse graphs. |
| [`algorithms/tarjan_scc.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algorithms/tarjan_scc.hpp) | `tarjan_scc(graph)` | `scc_result` | Linear-time $O(V + E)$ strongly connected component decomposition with topological sort. |
| [`algorithms/condensation.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algorithms/condensation.hpp) | `condensation_graph(graph, scc)` | `bidirectional_adjacency_graph` | Contracts SCCs into meta-vertices yielding an explicit condensation DAG. |
| [`algorithms/topological_sort.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algorithms/topological_sort.hpp) | `topological_sort(graph)` | `std::optional<std::vector>` | Kahn's algorithm for linear topological ordering with cycle detection. |
| [`algorithms/connectivity.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algorithms/connectivity.hpp) | `find_cut_vertices_and_bridges(graph)` | `biconnectivity_result` | Tarjan low-link articulation points (cut vertices) and bridges in $O(V + E)$ time. |
| [`algorithms/eulerian.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algorithms/eulerian.hpp) | `find_eulerian_trail_undirected`, `find_eulerian_trail_directed` | `eulerian_result` | Hierholzer's algorithm synthesizing complete Eulerian walks in $O(V + E)$ time. |
| [`algorithms/minimum_spanning_tree.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algorithms/minimum_spanning_tree.hpp) | `minimum_spanning_tree_kruskal`, `minimum_spanning_tree_prim` | `spanning_forest_result<W>` | MST and forest synthesis with acyclicity and connectivity invariant verification. |
| [`algorithms/shortest_paths.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algorithms/shortest_paths.hpp) | `dijkstra_shortest_paths`, `bellman_ford_shortest_paths`, `floyd_warshall_all_pairs` | `shortest_path_result<W>` | Single-source and all-pairs shortest paths with lazy path reconstruction and cycle detection. |
| [`algorithms/network_flow.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algorithms/network_flow.hpp) | `max_flow_dinic`, `max_flow_edmonds_karp`, `compute_cut_capacity` | `flow_result<Cap>` | Dinic layered blocking flows, Edmonds-Karp, flow conservation, and Max-Flow Min-Cut duality. |
| [`algorithms/bipartite_matching.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algorithms/bipartite_matching.hpp) | `maximum_bipartite_matching_hopcroft_karp`, `minimum_vertex_cover` | `matching_result` | Hopcroft-Karp $O(E\sqrt{V})$ matching and constructive realization of König's duality theorem ($|M| = |C|$). |

---

## 5. Automata & Formal Languages

| Header | Major Types | Key Functions | One-Line Summary |
| :--- | :--- | :--- | :--- |
| [`automata/dfa.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/automata/dfa.hpp) | `dfa` | `transition(q, a)`, `accepts(word)` | Total deterministic finite automata with flat transition table and $O(1)$ state stepping. |
| [`automata/nfa.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/automata/nfa.hpp) | `nfa` | `add_transition(q, a, p)`, `add_epsilon_transition(q, p)` | Nondeterministic finite automata with set-valued symbol transitions and spontaneous $\varepsilon$-moves. |
| [`automata/algorithms.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/automata/algorithms.hpp) | `state_set`, `dfa_minimization_result` | `subset_construction(nfa)`, `minimize_dfa(dfa)`, `is_language_equivalent(d1, d2)` | $\varepsilon$-closure, powerset determinization, Hopcroft state minimization, and decision procedures. |
| [`automata/regex.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/automata/regex.hpp) | `regex`, `regex_node` | `operator+`, `operator\|`, `star()`, `thompson_construction()`, `to_min_dfa()` | Pure AST regular expressions with fluent operator overloads and complete compilation pipelines. |

---

## 6. Logic & Constraint Satisfaction

| Header | Major Types | Key Functions | One-Line Summary |
| :--- | :--- | :--- | :--- |
| [`logic/formula.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/logic/formula.hpp) | `formula` | `var(i)`, `eval(assign)`, `equivalent(f1, f2)` | Pure AST propositional logic supporting boolean operators, $O(1)$ bitmask evaluation, and entailment. |
| [`logic/normal_forms.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/logic/normal_forms.hpp) | AST transforms | `to_nnf(f)`, `to_dnf(f)`, `to_cnf(f)`, `to_canonical_dnf(f)` | De Morgan and distributive normal form transformations. |
| [`logic/truth_table.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/logic/truth_table.hpp) | `truth_table` | `generate_truth_table(f)`, `is_tautology(f)` | Complete truth table generation with tautology, contradiction, and satisfiability checks. |
| [`logic/two_sat.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/logic/two_sat.hpp) | `formula_2cnf`, `clause2`, `literal` | `solve_2sat(f)`, `implication_graph(f)` | Aspvall-Plass-Tarjan linear-time 2-SAT solver using implication digraphs and SCC model extraction. |

---

## 7. Order Theory & Lattices

| Header | Major Types | Key Functions | One-Line Summary |
| :--- | :--- | :--- | :--- |
| [`order/poset.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/order/poset.hpp) | `poset` | `from_dag_closure(r)`, `hasse_diagram()`, `is_lattice()`, `linear_extension()` | Transitive reduction $C = \preceq \setminus (\preceq \circ \preceq)$, Hasse diagrams, topological extensions, and lattice meet/join testing. |

---

## 8. Abstract Algebra & Morphisms

| Header | Major Types | Key Functions | One-Line Summary |
| :--- | :--- | :--- | :--- |
| [`algebra/operation_table.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algebra/operation_table.hpp) | `operation_table<Dom>` | `from_callable(dom, f)`, `apply(i, j)` | Row-major Cayley multiplication tables over finite domains with $O(1)$ element lookups. |
| [`algebra/laws.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algebra/laws.hpp) | Law verification | `is_associative(t)`, `is_commutative(t)`, `find_identity(t)` | Algorithmic testing of algebraic axioms: associativity, commutativity, identity, invertibility, distributivity. |
| [`algebra/monoid.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algebra/monoid.hpp) | `finite_monoid<Dom>` | `identity()`, `op(a, b)` | Validated associative structures with verified two-sided identity elements. |
| [`algebra/group.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algebra/group.hpp) | `finite_group<Dom>` | `inverse(a)`, `element_order(a)`, `is_subgroup(elems)` | Finite groups with $O(1)$ element inverses, order calculation, and subgroup validation. |
| [`algebra/morphism.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algebra/morphism.hpp) | Morphism mappings | `is_homomorphism(g1, g2, m)`, `kernel(m)`, `is_isomorphism(g1, g2, m)` | Homomorphism verification, kernel and image extraction, and structural isomorphism testing. |
| [`algebra/boolean_algebra.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algebra/boolean_algebra.hpp) | `finite_boolean_algebra<Dom>` | `meet(a, b)`, `join(a, b)`, `complement(a)` | Finite Boolean algebras $(B, \lor, \land, \bar{\cdot}, \bot, \top)$ satisfying De Morgan complementation. |
| [`algebra/quotient.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algebra/quotient.hpp) | `quotient_result` | `is_normal_subgroup(g, h)`, `quotient_group(g, h)`, `certify_first_isomorphism_theorem()` | Conjugation invariance, coset partitions, quotient groups $G/H$, and First Isomorphism Theorem certification. |
| [`algebra/builders.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algebra/builders.hpp) | Builders | `cyclic_group(n)`, `unit_group_mod_n(n)`, `symmetric_group_3()` | Pre-built classical mathematical groups and cross-subsystem structural isomorphism bridges. |

---

## 9. Enumerative Combinatorics & Number Theory

| Header | Major Generators / Functions | Key Concepts | One-Line Summary |
| :--- | :--- | :--- | :--- |
| [`combinatorics/counting.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/combinatorics/counting.hpp) | `factorial`, `combinations_count`, `stirling_second`, `bell_number`, `catalan_number` | Exact 64-bit integer counts | Exact arithmetic counting functions with integer overflow detection. |
| [`combinatorics/subsets.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/combinatorics/subsets.hpp) | `k_subsets_view`, `power_set_view` | Lazy combinatoric views | Non-allocating ranges generating all $\binom{n}{k}$ subsets or $2^n$ power sets. |
| [`combinatorics/permutations.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/combinatorics/permutations.hpp) | `permutations_view` | Lexicographical permutations | Non-allocating generator yielding all $n!$ permutations in lexicographical order. |
| [`combinatorics/integer_partitions.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/combinatorics/integer_partitions.hpp) | `integer_partitions_view` | Partitions $\lambda \vdash n$ | Non-allocating generator for all non-increasing integer partitions summing to $n$. |
| [`combinatorics/set_partitions.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/combinatorics/set_partitions.hpp) | `set_partitions_view`, `k_set_partitions_view` | Restricted Growth Strings (RGS) | Canonically generates all $B_n$ or $S(n, k)$ set partitions as Restricted Growth Strings. |
| [`combinatorics/project.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/combinatorics/project.hpp) | `views::project_subsets`, `views::project_permutations` | Domain projections | Adapts index generators over arbitrary user domains using zero-overhead adapters. |
| [`number_theory/properties.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/number_theory/properties.hpp) | `extended_gcd`, `divisors`, `euler_totient`, `carmichael`, `solve_linear_diophantine` | Divisibility & Totients | Euclidean algorithm, linear Diophantine equations, Euler totient, and divisor collections. |
| [`number_theory/primes.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/number_theory/primes.hpp) | `is_prime_miller_rabin`, `sieve_of_eratosthenes`, `prime_factors` | Primality & Factorization | Deterministic Miller-Rabin primality testing for all 64-bit integers ($n < 2^{64}$). |
| [`number_theory/modular.hpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/number_theory/modular.hpp) | `dynamic_mod_int`, `power_mod`, `mod_inverse`, `chinese_remainder_theorem` | Modular arithmetic & CRT | Dynamic modular arithmetic rings, modular inverses, and general Chinese Remainder Theorem. |
