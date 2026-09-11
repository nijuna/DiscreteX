# DiscreteX v1.0.0 Release Notes

**Release Date:** September 2026  
**License:** MIT License  
**Language Standard:** ISO C++20 (`-std=c++20`)  
**Repository:** [https://github.com/nijuna/DiscreteX](https://github.com/nijuna/DiscreteX)

---

## 1. Release Highlights

DiscreteX v1.0.0 marks the first official public release of a modern, unified C++20 library for finite discrete mathematics and algorithmic structures. The release provides:

- **Complete Mathematical Scope**: 34 header files covering 9 core domains, fully implemented and tested without external dependencies.
- **Dual Representation Engine**: Dense bit-matrix storage accelerated by 64-bit hardware bit-scan instructions (`std::countr_zero`) paired with cache-friendly sparse adjacency lists.
- **Cross-Subsystem Bridges**: Formal mathematical transformations unifying propositional logic, automata theory, abstract algebra, order theory, and network optimization.
- **Production Packaging**: Full CMake 3.15+ support exporting `DiscreteX::DiscreteX` with `FetchContent`, package config helpers, and semantic version tracking.
- **Comprehensive Verification**: 25 unit test suites and 5 curated standalone examples compiled with 0 warnings under `-std=c++20 -Wall -Wextra -Wpedantic -O2`.

---

## 2. Supported Mathematical Domains

DiscreteX unifies the following domains under coherent C++20 concepts:

1. **Foundations & Core Domains (`discretex/core`)**: Dense integer domains (`index_domain`), semantic mapping layer (`mapped_domain<T>`), and Disjoint Set Union (`dsu`) with path compression and union-by-size.
2. **Binary Relations & Quotients (`discretex/relation`)**: Packed 64-bit bit-matrix (`dense_relation`), in-place Warshall transitive closure, quotient set extraction, equivalence relation verification, and zero-copy transpose views (`views::transpose`).
3. **Graph Algorithms & Connectivity (`discretex/graph`, `discretex/algorithms`)**:
   - Shortest paths: DAG topological relaxation, Dijkstra min-priority queue, Bellman-Ford signed weights with negative cycle detection, and Floyd-Warshall all-pairs distance matrices.
   - Minimum spanning trees: Kruskal (DSU) and Prim (priority queue) with structural invariant certification.
   - Network flow: Dinic blocking flow and Edmonds-Karp with Max-Flow Min-Cut duality and flow conservation checks.
   - Matching: Kuhn and Hopcroft-Karp maximum bipartite matching with König's theorem minimum vertex cover.
   - Connectivity: Tarjan linear-time Strongly Connected Components (SCC), condensation DAG, Tarjan low-link biconnectivity (bridges and cut vertices), and Hierholzer directed and undirected Eulerian trail synthesis.
4. **Enumerative Combinatorics (`discretex/combinatorics`)**: Exact 64-bit arithmetic for factorials, Stirling numbers ($S(n, k)$, $|c(n, k)|$), Bell numbers, Catalan numbers, and integer partition counts; non-allocating lazy views for $k$-subsets, permutations, power sets, and integer partitions.
5. **Order Theory & Lattices (`discretex/order`)**: Partially ordered sets (`poset`), covering relation reduction ($C = \preceq \setminus (\preceq \circ \preceq)$), Hasse diagram generation, topological linear extensions, and lattice meet/join verification.
6. **Propositional Logic & 2-SAT (`discretex/logic`)**: Pure AST formulas, bitmask assignment evaluations, truth tables, normal forms (NNF, DNF, CNF), and Aspvall-Plass-Tarjan linear-time 2-SAT solver over directed implication graphs.
7. **Number Theory & Modular Arithmetic (`discretex/number_theory`)**: Extended Euclidean algorithm, linear Diophantine equations, safe 64-bit modular arithmetic (`dynamic_mod_int`), Miller-Rabin deterministic primality testing ($n < 2^{64}$), prime factorization, divisor enumeration, Euler totient, and general Chinese Remainder Theorem.
8. **Abstract Algebra & Morphisms (`discretex/algebra`)**: Cayley operation tables, monoids, groups, element orders, homomorphism validation, kernel and image extraction, normal subgroup testing, coset partitions, quotient groups ($G / H$), and constructive certification of the First Isomorphism Theorem ($G / \ker f \cong \text{im} f$).
9. **Automata & Formal Languages (`discretex/automata`)**: Total DFAs, $\varepsilon$-NFAs with set-valued transitions, dense bitset state sets (`state_set`), Thompson inductive regex compilation, powerset determinization, Hopcroft partition refinement minimization, and decision procedures (`is_empty_language`, `is_universal_language`, `is_language_included`, `is_language_equivalent`).

---

## 3. Installation and Integration

DiscreteX is header-only and requires an ISO C++20 compliant compiler.

### Option A: CMake FetchContent (Recommended)
```cmake
include(FetchContent)

FetchContent_Declare(
    DiscreteX
    GIT_REPOSITORY https://github.com/nijuna/DiscreteX.git
    GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(DiscreteX)

target_link_libraries(my_app PRIVATE DiscreteX::DiscreteX)
```

### Option B: Pre-Installed Package
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
sudo cmake --install build
```

Downstream `CMakeLists.txt`:
```cmake
find_package(DiscreteX CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE DiscreteX::DiscreteX)
```

### Option C: Direct Include
```bash
g++ -std=c++20 -O2 -I/path/to/DiscreteX/include main.cpp -o my_app
```

---

## 4. Curated Standalone Examples

Five self-contained examples are located in the `examples/` directory:

- `examples/regex_to_min_dfa.cpp`: Inductive Thompson compilation, powerset determinization, Hopcroft minimization, and formal language equivalence proofs.
- `examples/quotient_group_isomorphism.cpp`: Permutation group $S_3$, normal subgroup $A_3$, quotient group $S_3 / A_3$, and programmatic certification of the First Isomorphism Theorem ($S_3 / A_3 \cong \mathbb{Z}_2$).
- `examples/network_flow_min_cut.cpp`: Dinic maximum flow, flow conservation checks, and Max-Flow Min-Cut duality verification.
- `examples/two_sat_solver.cpp`: Propositional 2-CNF formula translation into implication graphs, linear-time Tarjan SCC, and satisfying model extraction.
- `examples/shortest_paths.cpp`: Dijkstra shortest paths, lazy path reconstruction with weight integrity verification, and Floyd-Warshall distance matrices.

Execute all examples via:
```bash
make examples
```

---

## 5. Stability and API Guarantees

- **API Stability**: DiscreteX v1.0.0 establishes the baseline public API. All exported interfaces in `include/discretex/` adhere to Semantic Versioning (`SameMajorVersion` compatibility).
- **Compiler Compatibility**:
  - GCC 11 or higher
  - Clang 13 or higher
  - Microsoft Visual C++ 2019 (version 16.11+) or higher
- **Standard Conformance**: Strict ISO C++20 (`-std=c++20`). Zero third-party runtime dependencies.
