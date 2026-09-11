# DiscreteX

[![CI](https://github.com/nijuna/DiscreteX/actions/workflows/ci.yml/badge.svg)](https://github.com/nijuna/DiscreteX/actions/workflows/ci.yml)
[![Release](https://img.shields.io/badge/release-v1.0.0-blue.svg)](https://github.com/nijuna/DiscreteX/releases/tag/v1.0.0)
[![Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

DiscreteX is a modern C++20 header-only library for finite discrete mathematics and algorithmic structures. It uses a bridge-driven architecture to connect abstract algebra, automata theory, propositional logic, graph theory, order theory, and combinatorics through coherent reusable abstractions. Built around a two-tier domain identity model, 64-bit hardware-accelerated bit-matrices, zero third-party dependencies, and fully verified test coverage.

In the standard C++ ecosystem, discrete mathematics is fragmented: graph libraries, combinatorics utilities, logic engines, and number-theoretic algorithms often use conflicting abstractions, bespoke container types, and inconsistent indexing models. 

The goal of DiscreteX is to unify these domains into a mathematically coherent, high-performance C++20 library. In DiscreteX, structures are not isolated: partially ordered sets naturally produce directed acyclic graphs; propositional valuation spaces are isomorphic to Boolean lattices; 2-SAT satisfiability reduces to strongly connected components in implication graphs; integer partitions and set partitions connect directly with equivalence relations; and divisibility relations on integers form distributive lattices that share the exact same order-theoretic and graph algorithms.

---

## Documentation

- **[Start Here: 5-Minute Orientation](docs/start_here.md)**: Fast setup, 10-line sandbox demo, learning tracks, and recommended reading path.
- **[API Quick Reference Index](docs/api_index.md)**: Navigable reference of all 34 header files, types, algorithms, and one-line summaries.
- **[One-Page Project Summary](docs/project_summary.md)**: Compact briefing for academic outreach, course adoption, and portfolio use.
- **[Architectural Overview and System Design](docs/architecture.md)**: Comprehensive analysis of the two-tier domain identity model, 64-bit hardware-accelerated bit-matrices, concept abstractions, cross-subsystem bridges, and theoretical asymptotic bounds.
- **[Theory-to-Code Tour](docs/theory_to_code_tour.md)**: Guided walkthrough of the 5 primary mathematical pipelines mapping formal theorems directly to verified C++20 code.
- **[Release Notes v1.0.0](docs/release_notes_v1.0.0.md)**: Complete v1.0.0 release highlights, supported mathematical domain taxonomy, and API stability guarantees.

---

## Architectural Principles

### 1. Two-Tier Domain System
- **Dense Execution Identity (`index_domain`)**: Algorithms in the core execute over contiguous integer domains $[0, n)$. This guarantees maximum cache locality, array indexing, and zero mapping overhead.
- **Semantic Mapped Identity (`mapped_domain<T>`)**: Ingestion and client APIs map arbitrary user objects (strings, custom structs, integers) to contiguous indices once at the boundary, ensuring clean ergonomics without compromising inner loop speed.

### 2. Explicit C++20 Concepts
DiscreteX separates semantic relations from execution graphs via concepts:
- `concepts::FiniteDomain` and `concepts::IndexableDomain<T>`
- `concepts::Relation`
- `concepts::ForwardRelation` (requires `out_neighbors(u)`)
- `concepts::BidirectionalRelation` (requires `in_neighbors(u)`)
- `concepts::ForwardGraph` and `concepts::BidirectionalGraph`

### 3. Dual Storage Engines & Zero-Copy Views
- **Dense Bit-Matrix (`dense_relation`)**: Stores relations as packed 64-bit words. Row fibers are traversed via `bit_row_fiber_view` using `std::countr_zero` hardware instructions to skip empty 64-bit words in $O(1)$. In-place transitive closures run via optimized bitwise Warshall operations.
- **Sparse Adjacency Lists (`forward_adjacency_graph`, `bidirectional_adjacency_graph`)**: Maintains sorted neighbor vectors for $O(\\log \\text{deg})$ edge queries and amortized $O(E \\log \\text{deg})$ bulk loading via `from_edges`.
- **Zero-Copy Views (`views::transpose`)**: Provides converse relations $R^{-1}$ at zero allocation cost by flipping forward and backward neighbor accessors.

---

## Implemented Subsystems

### 1. Core Relations, Partitions & Disjoint Sets (`discretex/core`, `discretex/relation`)
- Mathematical binary relations over finite domains with packed 64-bit word storage and bit-scan fiber traversal.
- Equivalence relation axiom verification (`is_equivalence_relation`) requiring only `concepts::Relation`.
- Canonical quotient set extraction (`equivalence_classes_rgs`, `equivalence_classes`, `quotient_size`).
- Two-way bridge between set partitions and dense equivalence relations (`relation_from_partition`).
- In-place Warshall transitive closure.
- Generic Breadth-First Search (`algorithms::bfs`) operating uniformly across dense relations, sparse graphs, and transpose views.
- **Disjoint Set Union (`core/dsu.hpp`)**:
  - `disjoint_set` (alias `dsu`) over `index_domain` with path compression and union by size in amortized $O(\alpha(n))$ time.
  - Component tracking (`component_count`, `component_size`, `components`) and equivalence partition export (`to_rgs`).

### 2. Graph Algorithms & Structural Connectivity (`discretex/algorithms`, `discretex/graph`)
- **Flow Networks (`graph/flow_network.hpp`)**:
  - Dedicated semantic network `flow_network<Capacity, Dom>` maintaining directed capacity edges alongside linked residual edge pairs (`residual_edge<Capacity>`).
  - Constant-time $O(1)$ reverse residual updates via mutual `rev` index tracking.
  - Retains immutable original network definition with support for parallel edges and self-loops.
- **Maximum Flow & Minimum Cut (`algorithms/network_flow.hpp`)**:
  - `max_flow_edmonds_karp`: Augmenting path algorithm via BFS in $O(|V| \cdot |E|^2)$.
  - `max_flow_dinic`: Layered BFS level-graph construction with DFS blocking flows and current-edge pointer optimization in $O(|V|^2 |E|)$ (and $O(|E|\sqrt{|V|})$ on unit networks).
  - `flow_result<Capacity>`: Reports maximum flow, source-side minimum cut indicator ($S \subseteq V$), and per-edge flow values.
  - Invariants: Flow conservation verification (`verify_flow_conservation`) and Max-Flow Min-Cut duality verification (`compute_cut_capacity`).
  - Reduction bridge: Exact reduction of bipartite matching to unit flow networks, recovering matching size $|M|$ and König's minimum vertex cover $|C|$.
- **Weighted Graphs (`graph/weighted_graph.hpp`)**:
  - Structured `weighted_edge<Weight>` and `weighted_neighbor<Weight>` supporting arbitrary ordered weight types.
  - `weighted_directed_graph<Weight, Dom>` and `weighted_undirected_graph<Weight, Dom>` with dual flat-edge list and adjacency-list models.
  - $O(1)$ degrees, optional edge queries (`edge_weight`), and vertex/edge counts.
- **Shortest Paths (`algorithms/shortest_paths.hpp`)**:
  - `dag_shortest_paths`: Topological order relaxation running in $O(|V| + |E|)$, supporting negative edge weights on DAGs.
  - `dijkstra_shortest_paths`: Min-priority queue single-source shortest paths in $O((|V| + |E|) \log |V|)$ for non-negative weights.
  - `bellman_ford_shortest_paths`: Single-source shortest paths in $O(|V| \cdot |E|)$ supporting arbitrary signed weights and detecting reachable negative cycles.
  - `floyd_warshall_all_pairs`: Dynamic programming all-pairs shortest paths in $O(|V|^3)$ with next-hop matrices and negative cycle detection.
  - Results and Reconstruction: `shortest_path_result`, `bellman_ford_result`, and `all_pairs_shortest_path_result` using `std::optional<Weight>` distances, with lazy path reconstruction (`reconstruct_path`) and integrity verification (`verify_path_integrity`).
- **Minimum Spanning Trees & Forests (`algorithms/minimum_spanning_tree.hpp`)**:
  - `minimum_spanning_tree_kruskal`: Kruskal's algorithm in $O(E \log E + E \alpha(V))$ using DSU.
  - `minimum_spanning_tree_prim`: Prim's algorithm in $O(E \log V)$ using min-priority queues.
  - Returns `spanning_forest_result<Weight>` with edge list, total weight, component count, and connectivity flag.
  - `is_valid_spanning_forest`: Verifies structural invariants: acyclicity, exact edge count $|E| = |V| - c$, edge membership, and connectivity equivalence.
  - `connected_components` and `connected_component_count` using DSU.
- **Bipartite Graphs (`graph/bipartite_graph.hpp`)**:
  - Explicit partitioned graph structure $G = (L \cup R, E)$ with zero-overhead side-local vertex indices.
  - Sorted neighbor arrays with $O(\log \text{deg})$ edge queries and `std::span` neighbor access.
- **Maximum Bipartite Matching (`algorithms/bipartite_matching.hpp`)**:
  - `maximum_bipartite_matching_kuhn`: Augmenting path algorithm in $O(|V| \cdot |E|)$.
  - `maximum_bipartite_matching_hopcroft_karp`: Layered BFS/DFS augmenting path algorithm running in $O(|E|\sqrt{|V|})$.
  - `matching_result`: Exposes left-to-right and right-to-left mate arrays, total cardinality $|M|$, and matched edge pairs.
- **König's Theorem Minimum Vertex Cover (`algorithms/bipartite_matching.hpp`)**:
  - `minimum_vertex_cover`: Constructive realization of König's duality theorem ($|M| = |C|$) on bipartite graphs.
  - Computes reachable alternating components $(Z_L, Z_R)$ from unmatched left vertices to construct the exact minimal vertex cover $C = (L \setminus Z_L) \cup Z_R$.
- **Tarjan Strongly Connected Components (`algorithms/tarjan_scc.hpp`)**:
  - Computes SCC decomposition in linear time $O(V + E)$ on any `concepts::ForwardRelation`.
  - Normalizes component IDs in topological order of the condensation DAG.
  - Returns `scc_result` providing both $O(1)$ vertex component lookup (`component_of[v]`) and explicit component member lists (`components[c]`).
- **Condensation DAG (`algorithms/condensation.hpp`)**:
  - Contracts each strongly connected component into a meta-vertex in `index_domain(component_count)`.
  - Generates an acyclic `bidirectional_adjacency_graph` with automatically sorted, deduplicated cross-component edges.
- **Topological Sorting (`algorithms/topological_sort.hpp`)**:
  - Kahn's algorithm with cycle detection over any `concepts::ForwardRelation`.
- **Biconnectivity, Cut Vertices & Bridges (`algorithms/connectivity.hpp`)**:
  - `find_cut_vertices_and_bridges`: Tarjan's low-link algorithm running in $O(V + E)$ on undirected graphs.
  - Returns `biconnectivity_result` containing articulation points (cut vertices) and bridges.
  - Helpers: `find_cut_vertices` and `find_bridges`.
- **Eulerian Circuits & Trails (`algorithms/eulerian.hpp`)**:
  - Dedicated undirected predicates and tour synthesis: `has_eulerian_circuit_undirected`, `has_eulerian_trail_undirected`, and `find_eulerian_trail_undirected` via Hierholzer's algorithm in $O(V + E)$.
  - Dedicated directed predicates and tour synthesis: `has_eulerian_circuit_directed`, `has_eulerian_trail_directed`, and `find_eulerian_trail_directed`.
  - Returns `eulerian_result` containing the ordered vertex sequence of the Eulerian walk.

### 3. Enumerative Combinatorics & Generators (`discretex/combinatorics`)
- **Counting & Special Numbers (`counting.hpp`)**:
  - Exact 64-bit arithmetic with overflow detection (`factorial`, `falling_factorial`, `combinations_count`).
  - Stirling numbers of the second kind ($S(n, k)$) and unsigned Stirling numbers of the first kind ($|c(n, k)|$).
  - Bell numbers ($B_n$), Catalan numbers ($C_n$), and unrestricted partition numbers ($p(n)$ via Euler's pentagonal theorem).
- **Subsets & Power Sets**:
  - `k_subsets_view`: all $\\binom{n}{k}$ combinations in lexicographical order.
  - `power_set_view`: safe generation for $n \\le 63$ in binary counting or reflected Gray code.
- **Permutations & Integer Partitions**:
  - `permutations_view`: all $n!$ permutations in lexicographical order yielding `std::span<const std::size_t>`.
  - `integer_partitions_view`: all non-increasing partitions $\\lambda \\vdash n$ summing to $n$.
- **Set Partitions via Restricted Growth Strings (RGS)**:
  - `set_partitions_view`: all $B_n$ set partitions represented canonically as RGS.
  - `k_set_partitions_view`: exact direct generation of $S(n, k)$ partitions into $k$ blocks.
- **Domain Projection**:
  - `views::project_subsets` and `views::project_permutations` adapting index generators to user domains.

### 4. Order Theory & Lattices (`discretex/order`)
- Closure-backed partially ordered sets (`poset`).
- Exact covering relation reduction: $C = S \\setminus (S \\circ S)$.
- Hasse diagram construction (`poset::hasse_diagram`).
- Topological linear extensions (`poset::linear_extension`) via Kahn's algorithm.
- Lattice testing (`poset::is_lattice`), meets, and joins.
- Strict constructor validation (`from_dag_closure` vs. `from_hasse_diagram` rejecting transitive shortcuts).

### 5. Propositional Logic & 2-SAT Solver (`discretex/logic`)
- Pure AST formula representation (`formula`) supporting variables, negation, conjunction, disjunction, implication, and biconditional equivalence.
- Bitmask formula evaluation over 64-bit assignments ($O(1)$ variable lookup).
- Truth table generator (`truth_table`) with tautology, contradiction, and satisfiability checking.
- Semantic equivalence (`equivalent`) and logical entailment (`entails`).
- Normal forms:
  - Negation Normal Form (`to_nnf`) via De Morgan's laws and double-negation elimination.
  - Algebraic DNF & CNF (`to_dnf`, `to_cnf`) via distributive laws.
  - Canonical DNF & CNF (`to_canonical_dnf`, `to_canonical_cnf`) via minterm and maxterm expansions.
- **2-SAT & Implication Graphs (`two_sat.hpp`)**:
  - Structured `literal` and 2-clause representation (`clause2`, `formula_2cnf`).
  - Direct implication graph construction ($(\\neg a \\to b) \\land (\\neg b \\to a)$).
  - Aspvall-Plass-Tarjan linear-time 2-SAT solver (`solve_2sat`) with model extraction via topological SCC ordering.
- **Valuation Space Bridge**: Direct isomorphism between satisfying valuation subsets and Boolean lattice operations ($\\cap \\leftrightarrow \\land$, $\\cup \\leftrightarrow \\lor$, $\\setminus \\leftrightarrow \\neg$, $\\subseteq \\leftrightarrow \\models$).

### 6. Number Theory & Modular Arithmetic (`discretex/number_theory`)
- Euclidean and Extended Euclidean algorithms (`extended_gcd`).
- Linear Diophantine equation solver ($ax + by = c$).
- Safe 64-bit modular arithmetic (`add_mod`, `sub_mod`, `mul_mod`, `power_mod`, `mod_inverse`).
- Dynamic modular rings (`dynamic_mod_int`).
- Primality testing: deterministic Miller-Rabin test for all 64-bit integers ($n < 2^{64}$).
- Sieve of Eratosthenes (`sieve_of_eratosthenes`).
- Canonical prime factorization (`prime_factors`).
- Divisor enumeration (`divisors`).
- Euler's totient function (`euler_totient`) and Carmichael function (`carmichael`).
- Linear congruence solver ($ax \\equiv b \\pmod m$).
- General Chinese Remainder Theorem (`chinese_remainder_theorem`) supporting both pairwise coprime and non-coprime moduli.
- **Divisibility Lattice Bridge**: Proof that the divisibility poset $D_n = (\\{d \\mid n\\}, \\mid)$ forms a distributive lattice with meet $\\gcd(u, v)$ and join $\\text{lcm}(u, v)$, whose Hasse diagram for square-free $n$ is isomorphic to the hypercube graph $Q_k$.

### 7. Abstract Algebra & Morphisms (`discretex/algebra`)
- **Operation Tables (`algebra/operation_table.hpp`)**:
  - Flat $n \\times n$ row-major Cayley tables over finite domains with $O(1)$ lookups.
  - Construction from callables (`from_callable`) with boundary closure validation.
- **Algebraic Law Verification (`algebra/laws.hpp`)**:
  - Single operation laws: `is_associative` ($O(n^3)$), `is_commutative`, `find_identity`, `has_identity`, `inverse_table`, `has_inverses`, and `is_idempotent`.
  - Dual operation laws: `is_distributive`, `is_absorptive`, and `satisfies_boolean_complements`.
- **Finite Monoids & Groups (`algebra/monoid.hpp`, `algebra/group.hpp`)**:
  - `finite_monoid`: Validated associative structures with two-sided identity elements.
  - `finite_group`: Invertible monoids with $O(1)$ element inverse lookups, `is_abelian`, element orders (`element_order`), and subgroup verification (`is_subgroup`).
- **Morphisms & Isomorphisms (`algebra/morphism.hpp`)**:
  - Explicit mapping representations `std::vector<std::size_t>`.
  - Verification of operation preservation (`is_homomorphism`), injectivity, surjectivity, and bijectivity.
  - Structure isomorphism testing (`is_isomorphism`), kernel extraction (`kernel`), and image ranges (`image`).
- **Finite Boolean Algebras (`algebra/boolean_algebra.hpp`)**:
  - Algebraic structures $(B, \\lor, \\land, \\bar{\\cdot}, \\bot, \\top)$ with verified lattice absorption, mutual distributivity, and De Morgan complementation.
- **Quotient Groups & First Isomorphism Theorem (`algebra/quotient.hpp`)**:
  - `is_normal_subgroup`: Verifies subgroup closure and conjugation invariance ($g \\cdot h \\cdot g^{-1} \\in H$).
  - `coset_partition`: Partitions group elements into disjoint left cosets $gH$, extracting member sets and canonical $O(1)$ projection maps.
  - `quotient_group`: Constructs the quotient group $G / H$ as a first-class `finite_group<index_domain>` with verified coset multiplication $(aH)(bH) = (ab)H$.
  - `certify_first_isomorphism_theorem`: Constructive realization of the First Isomorphism Theorem for Groups ($G / \ker(f) \cong \text{im}(f)$), certifying structure-preserving bijections between cosets and image elements.
- **Builders & Cross-Subsystem Bridges (`algebra/builders.hpp`)**:
  - `cyclic_group(n)`: Additive cyclic group $(\mathbb{Z}/n\mathbb{Z}, +)$.
  - `unit_group_mod_n(n)`: Multiplicative group of units $(\mathbb{Z}/n\mathbb{Z})^\times$.
  - `klein_four_group()`: Klein four-group $V_4 \cong \mathbb{Z}_2 \times \mathbb{Z}_2$.
  - `power_set_boolean_algebra(k)`: Boolean algebra $\mathcal{P}(\{0, \dots, k-1\})$ of size $2^k$.
  - Structural isomorphism bridge: Certified isomorphism $(\mathbb{Z}/8\mathbb{Z})^\times \cong V_4$.
  - First Isomorphism Theorem bridge: Certified quotient isomorphisms $\mathbb{Z}_6 / \{0, 3\} \cong \mathbb{Z}_3$, $\mathbb{Z}_{12} / \{0, 4, 8\} \cong \mathbb{Z}_4$, and $S_3 / A_3 \cong \mathbb{Z}_2$.

### 8. Automata & Formal Languages (`discretex/automata`)
- **Deterministic Finite Automata (`automata/dfa.hpp`)**:
  - Dense total transition table of size $|Q| \times |\Sigma|$ (`transitions_[q * \Sigma + a]`) with $O(1)$ state transitions.
  - Totality enforcement ensuring every state has defined transitions for every symbol in the alphabet.
  - Start state configuration, characteristic boolean vector of accepting states, and generic word acceptance (`accepts`).
- **Nondeterministic Finite Automata (`automata/nfa.hpp`)**:
  - Sparse set-valued symbol transitions (`transitions(q, a)`) returning `std::span<const std::size_t>`.
  - Separate representation of spontaneous $\varepsilon$-transitions (`epsilon_transitions(q)`).
  - Single initial start state, accepting states, and dynamic edge addition (`add_transition`, `add_epsilon_transition`).
- **Structural Automata Algorithms & Bridges (`automata/algorithms.hpp`)**:
  - `state_set`: Dense 64-bit word packed bitset representation for state subsets with three-way comparison (`<=>`) and set unions.
  - $\varepsilon$-Closure (`epsilon_closure`, `epsilon_closure_set`): BFS reachability over $\varepsilon$-edges from single states and arbitrary state collections.
  - Direct NFA Acceptance (`accepts`): Step-by-step subset tracking with transitive $\varepsilon$-closure evaluation.
  - Subset Construction (`subset_construction`, `determinize`): Powerset construction producing canonical total DFAs alongside inspectable subset mappings (`dfa_state_to_nfa_subset`). Certified language equivalence between NFA and determinized DFA.
  - Complementation (`complement`): Fast $O(|Q|)$ inversion of accepting states on total DFAs, with involution invariance ($\overline{\overline{D}} \cong D$).
  - Product Intersection (`intersect`): Direct synchronous product automaton on $Q_1 \times Q_2$ with reachability trimming, certifying $L(D_1 \cap D_2) = L(D_1) \cap L(D_2)$.
  - Reachability Trimming (`trim_unreachable`): Breadth-first elimination of unreachable states and dense renumbering to $[0, k)$.
  - Hopcroft Partition Refinement Minimization (`minimize_dfa`, `minimize_dfa_with_partition`):
    - Computes minimal quotient DFA in $O(|\Sigma| \cdot |Q| \log |Q|)$ time using inverse transition precomputation and partition splitting.
    - Produces canonical state numbering with start block indexed at 0 and remaining blocks ordered by minimum element.
    - **Equivalence Relation Bridge**: Minimization partition blocks are certified as a mathematical equivalence relation via `relation_from_partition`, directly connecting automata minimization to the quotient and partition algebra subsystem.
  - Language Equivalence (`language_equivalent`, `is_language_equivalent`): Product state space exploration verifying language identity between arbitrary DFAs.
  - Language Decision Procedures:
    - `is_empty_language`: Reachability verification of accepting states in $O(|Q| \cdot |\Sigma|)$ on DFAs and NFAs ($L = \emptyset$).
    - `is_universal_language`: Full state acceptance verification on total DFAs ($L(D) = \Sigma^*$).
    - `is_language_included`: Product state space exploration verifying $L(D_1) \subseteq L(D_2) \iff L(D_1) \cap \overline{L(D_2)} = \emptyset$.
- **Regular Expressions & Compilation Pipeline (`automata/regex.hpp`)**:
  - Pure AST node hierarchy (`regex`, `regex_node`, `regex_op`) supporting $\emptyset$, $\varepsilon$, literals, concatenation, alternation, and Kleene star.
  - Fluent algebraic operator overloads: `operator+` for concatenation and `operator|` for alternation.
  - Thompson Inductive Construction (`thompson_construction`, `to_nfa`): Inductive fragment composition with single start and single accept endpoints compiling any regex into an equivalent $\varepsilon$-NFA.
  - Closed-Loop Pipeline (`to_dfa`, `to_min_dfa`): Complete compilation chain $\text{Regex} \xrightarrow{\text{Thompson}} \text{NFA} \xrightarrow{\text{Powerset}} \text{DFA} \xrightarrow{\text{Hopcroft}} \text{DFA}_{\min}$.

---

## Integration and Installation

DiscreteX is a header-only library requiring ISO C++20. It has zero external dependencies beyond the standard library.

### Option 1: CMake FetchContent (Recommended)

Incorporate DiscreteX directly into your CMake project without manual installation:

```cmake
include(FetchContent)

FetchContent_Declare(
    DiscreteX
    GIT_REPOSITORY https://github.com/nijuna/DiscreteX.git
    GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(DiscreteX)

add_executable(my_project main.cpp)
target_link_libraries(my_project PRIVATE DiscreteX::DiscreteX)
```

### Option 2: System-Wide Installation

Install headers and CMake package configuration files:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
sudo cmake --install build
```

Then consume it in any downstream `CMakeLists.txt`:

```cmake
find_package(DiscreteX CONFIG REQUIRED)

add_executable(my_project main.cpp)
target_link_libraries(my_project PRIVATE DiscreteX::DiscreteX)
```

### Option 3: Direct Header Inclusion

Because DiscreteX is header-only, you can clone or copy the `include/` directory and add it to your compiler's include search path:

```bash
g++ -std=c++20 -O2 -I/path/to/DiscreteX/include main.cpp -o my_project
```

---

## Standalone Examples

DiscreteX includes curated, standalone examples in `examples/` demonstrating core mathematical algorithms and cross-subsystem bridges:

| Example Source | Subsystems Demonstrated | Key Invariants Verified |
| :--- | :--- | :--- |
| [`examples/regex_to_min_dfa.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/regex_to_min_dfa.cpp) | Automata, Regular Expressions | Thompson NFA, Powerset DFA, Hopcroft minimization, language inclusion and equivalence |
| [`examples/quotient_group_isomorphism.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/quotient_group_isomorphism.cpp) | Abstract Algebra | $S_3$ permutation group, normality checks, coset quotient groups, First Isomorphism Theorem ($S_3/A_3 \cong \mathbb{Z}_2$) |
| [`examples/network_flow_min_cut.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/network_flow_min_cut.cpp) | Graph Theory, Network Flow | Dinic blocking flow, flow conservation at all vertices, Max-Flow Min-Cut duality theorem |
| [`examples/two_sat_solver.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/two_sat_solver.cpp) | Logic, Graph Connectivity | 2-CNF implication graphs, Tarjan SCC decomposition, Aspvall-Plass-Tarjan linear 2-SAT solver, model extraction |
| [`examples/shortest_paths.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/shortest_paths.cpp) | Graph Theory, Shortest Paths | Dijkstra single-source shortest paths, lazy path reconstruction, path integrity certification, Floyd-Warshall |

Run all examples via:

```bash
make examples
```

---

## Build and Verification

### Requirements
- ISO C++20 compliant compiler:
  - GCC 11+
  - Clang 13+
  - MSVC 19.29+ (Visual Studio 2019 version 16.11+)
- Build systems: GNU Make or CMake 3.15+

### Build Targets

```bash
# Build and execute all 25 unit test suites and all 5 examples
make

# Execute only unit test suites
make test

# Compile and execute standalone examples
make examples

# Clean all build artifacts
make clean
```

All algorithms, models, and cross-subsystem bridges are compiled with `-std=c++20 -Wall -Wextra -Wpedantic -O2` and pass with 0 warnings.

---

## Repository Structure

```
DiscreteX/
├── .github/
│   └── workflows/
│       └── ci.yml
├── CMakeLists.txt
├── Makefile
├── README.md
├── LICENSE
├── cmake/
│   └── DiscreteXConfig.cmake.in
├── docs/
│   ├── api_index.md
│   ├── architecture.md
│   ├── project_summary.md
│   ├── release_notes_v1.0.0.md
│   ├── start_here.md
│   └── theory_to_code_tour.md
├── examples/
│   ├── network_flow_min_cut.cpp
│   ├── quotient_group_isomorphism.cpp
│   ├── regex_to_min_dfa.cpp
│   ├── shortest_paths.cpp
│   └── two_sat_solver.cpp
├── include/
│   └── discretex/
│       ├── discretex.hpp
│       ├── algebra/
│       │   ├── boolean_algebra.hpp
│       │   ├── builders.hpp
│       │   ├── group.hpp
│       │   ├── laws.hpp
│       │   ├── monoid.hpp
│       │   ├── morphism.hpp
│       │   ├── operation_table.hpp
│       │   └── quotient.hpp
│       ├── algorithms/
│       │   ├── bfs.hpp
│       │   ├── bipartite_matching.hpp
│       │   ├── condensation.hpp
│       │   ├── connectivity.hpp
│       │   ├── eulerian.hpp
│       │   ├── minimum_spanning_tree.hpp
│       │   ├── network_flow.hpp
│       │   ├── shortest_paths.hpp
│       │   ├── tarjan_scc.hpp
│       │   └── topological_sort.hpp
│       ├── automata/
│       │   ├── algorithms.hpp
│       │   ├── dfa.hpp
│       │   ├── nfa.hpp
│       │   └── regex.hpp
│       ├── combinatorics/
│       │   ├── counting.hpp
│       │   ├── integer_partitions.hpp
│       │   ├── permutations.hpp
│       │   ├── project.hpp
│       │   ├── set_partitions.hpp
│       │   └── subsets.hpp
│       ├── concepts/
│       │   ├── domain.hpp
│       │   ├── graph.hpp
│       │   └── relation.hpp
│       ├── core/
│       │   ├── bit_row_fiber_view.hpp
│       │   ├── domain.hpp
│       │   └── dsu.hpp
│       ├── graph/
│       │   ├── bipartite_graph.hpp
│       │   ├── flow_network.hpp
│       │   ├── sparse_graph.hpp
│       │   └── weighted_graph.hpp
│       ├── logic/
│       │   ├── formula.hpp
│       │   ├── normal_forms.hpp
│       │   ├── truth_table.hpp
│       │   └── two_sat.hpp
│       ├── number_theory/
│       │   ├── modular.hpp
│       │   ├── primes.hpp
│       │   └── properties.hpp
│       ├── order/
│       │   └── poset.hpp
│       └── relation/
│           ├── dense_relation.hpp
│           ├── equivalence.hpp
│           └── transpose_view.hpp
└── tests/
    ├── test_main.cpp
    └── test_runner.hpp
```

---

## License

DiscreteX is released under the MIT License.
