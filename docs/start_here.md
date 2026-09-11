# Start Here: A 5-Minute Orientation to DiscreteX

Welcome to DiscreteX. Whether you are an educator, researcher, student, or algorithm engineer, this guide provides a direct, minimal path from initial setup to running your first mathematical computation.

---

## 1. The 30-Second Mental Model

DiscreteX is built around one governing principle: **discrete mathematical structures are not isolated**.

Instead of treating graphs, logic formulas, relations, and automata as disconnected entities, DiscreteX connects them through constructive transformations (bridges):

```
Propositional 2-CNF  ───>  Implication Digraph  ───>  Tarjan SCC  ───>  Satisfying Model
Regex AST           ───>  Epsilon-NFA          ───>  Powerset DFA ───>  Minimal Quotient DFA
Integer Divisors    ───>  Covering Relation     ───>  Hasse Poset  ───>  Distributive Lattice
Group & Subgroup    ───>  Coset Partition       ───>  Quotient G/H ───>  First Isomorphism Thm
```

All algorithmic execution runs over dense, cache-friendly integer indices (`index_domain`), while arbitrary user entities (strings, coordinates, custom objects) are mapped once at the boundary via `mapped_domain<T>`.

---

## 2. Who This Library Is For

- **Educators & Instructors**: Teaching courses in discrete mathematics, automata theory, abstract algebra, or graph algorithms. DiscreteX provides executable, verified computational models that make formal definitions (e.g., quotient groups, Myhill-Nerode partitions, König's theorem) concrete.
- **Researchers & Mathematicians**: Prototyping finite algebraic structures, analyzing partially ordered sets and distributive lattices, testing language equivalence, and evaluating graph connectivity invariants.
- **Advanced Students**: Exploring how theoretical proofs translate directly into modern C++20 algorithms, concepts, and zero-allocation views.
- **Algorithm & Systems Engineers**: Integrating cache-friendly, hardware-accelerated relation bit-matrices (`std::countr_zero`), network flows, 2-SAT solvers, and shortest paths without introducing third-party dependencies.

---

## 3. Fast Setup (Zero External Dependencies)

DiscreteX requires an ISO C++20 compliant compiler (GCC 11+, Clang 13+, or MSVC 19.29+).

### Option A: Direct Header Inclusion (Fastest for experimentation)
Clone the repository and compile directly with `-Iinclude`:

```bash
git clone https://github.com/nijuna/DiscreteX.git
cd DiscreteX
g++ -std=c++20 -O2 -Iinclude your_program.cpp -o your_program
```

### Option B: CMake FetchContent (For CMake projects)
Add to your `CMakeLists.txt`:

```cmake
include(FetchContent)
FetchContent_Declare(
    DiscreteX
    GIT_REPOSITORY https://github.com/nijuna/DiscreteX.git
    GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(DiscreteX)

target_link_libraries(your_project PRIVATE DiscreteX::DiscreteX)
```

---

## 4. Your First 10-Line Sandbox Program

Create a file named `sandbox.cpp` to verify your environment and experience how DiscreteX combines relations, equivalence classes, and quotients:

```cpp
#include <iostream>
#include <discretex/discretex.hpp>

int main() {
    using namespace discretex;

    // 1. Create a binary relation over index domain {0, 1, 2, 3}
    index_domain dom(4);
    dense_relation r(dom);

    // Add base equivalence connections
    for (std::size_t i = 0; i < 4; ++i) r.add_pair(i, i); // Reflexivity
    r.add_pair(0, 1); r.add_pair(1, 0);                   // Block {0, 1}
    r.add_pair(2, 3); r.add_pair(3, 2);                   // Block {2, 3}

    // 2. Transitive closure in-place via word-level bitwise operations
    r.transitive_closure_inplace();

    // 3. Verify equivalence axioms and extract quotient classes
    bool is_equiv = is_equivalence_relation(r);
    std::cout << "Satisfies Equivalence Relation Axioms: " << (is_equiv ? "YES" : "NO") << "\n";

    auto classes = equivalence_classes(r);
    std::cout << "Quotient size |Domain / R| = " << classes.size() << "\n";
    for (std::size_t i = 0; i < classes.size(); ++i) {
        std::cout << "  Class " << i << ": { ";
        for (std::size_t x : classes[i]) std::cout << x << " ";
        std::cout << "}\n";
    }

    return 0;
}
```

Compile and run:
```bash
g++ -std=c++20 -O2 -Iinclude sandbox.cpp -o sandbox
./sandbox
```

Expected output:
```text
Satisfies Equivalence Relation Axioms: YES
Quotient size |Domain / R| = 2
  Class 0: { 0 1 }
  Class 1: { 2 3 }
```

---

## 5. Choose Your Track

Explore the library through domain-specific pathways:

### Track 1: Graph Theory & Network Flows
- **Core headers**: [`include/discretex/graph/`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/graph/), [`include/discretex/algorithms/`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algorithms/)
- **Key functions**: `dijkstra_shortest_paths`, `max_flow_dinic`, `minimum_spanning_tree_kruskal`, `find_cut_vertices_and_bridges`, `find_eulerian_trail_undirected`.
- **Reference example**: [`examples/network_flow_min_cut.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/network_flow_min_cut.cpp)

### Track 2: Formal Languages & Automata Synthesis
- **Core headers**: [`include/discretex/automata/`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/automata/)
- **Key functions**: `thompson_construction`, `subset_construction`, `minimize_dfa`, `is_language_equivalent`, `is_universal_language`.
- **Reference example**: [`examples/regex_to_min_dfa.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/regex_to_min_dfa.cpp)

### Track 3: Propositional Logic & Constraint Satisfaction
- **Core headers**: [`include/discretex/logic/`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/logic/)
- **Key functions**: `formula`, `truth_table`, `to_nnf`, `to_cnf`, `solve_2sat`.
- **Reference example**: [`examples/two_sat_solver.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/two_sat_solver.cpp)

### Track 4: Abstract Algebra & Quotient Morphisms
- **Core headers**: [`include/discretex/algebra/`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/algebra/)
- **Key functions**: `operation_table`, `finite_group`, `is_normal_subgroup`, `quotient_group`, `certify_first_isomorphism_theorem`.
- **Reference example**: [`examples/quotient_group_isomorphism.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/quotient_group_isomorphism.cpp)

### Track 5: Order Theory & Lattices
- **Core headers**: [`include/discretex/order/`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/order/)
- **Key functions**: `poset::from_dag_closure`, `poset::hasse_diagram`, `poset::linear_extension`, `poset::is_lattice`.
- **Reference in Theory Tour**: [`docs/theory_to_code_tour.md#4-order-theory-and-lattice-structures-divisibility-posets`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/docs/theory_to_code_tour.md)

### Track 6: Combinatorics & Number Theory
- **Core headers**: [`include/discretex/combinatorics/`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/combinatorics/), [`include/discretex/number_theory/`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/include/discretex/number_theory/)
- **Key functions**: `k_subsets_view`, `permutations_view`, `power_set_view`, `extended_gcd`, `is_prime_miller_rabin`, `chinese_remainder_theorem`.

---

## 6. Best First Example by User Profile

If you want to start with a full standalone example, choose based on your focus:

| Profile | Recommended Example | Key Concept Demonstrated |
| :--- | :--- | :--- |
| **Easiest Entry Point** | [`examples/shortest_paths.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/shortest_paths.cpp) | Dijkstra single-source paths, lazy path reconstruction, and automated integrity validation. |
| **Most Mathematical Bridge** | [`examples/quotient_group_isomorphism.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/quotient_group_isomorphism.cpp) | Non-abelian group $S_3$, normal subgroup checks, coset partitioning, and First Isomorphism Theorem ($S_3/A_3 \cong \mathbb{Z}_2$). |
| **Most Algorithmic / Systems** | [`examples/network_flow_min_cut.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/network_flow_min_cut.cpp) | Dinic layered blocking flows, Kirchhoff conservation checks, and Max-Flow Min-Cut duality. |
| **Most Unifying Pipeline** | [`examples/regex_to_min_dfa.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/regex_to_min_dfa.cpp) | Regex AST $\to$ Thompson NFA $\to$ Powerset DFA $\to$ Hopcroft Min DFA $\to$ Language Decision Procedures. |
| **Most Elegant Logic Reduction** | [`examples/two_sat_solver.cpp`](file:///media/Shared/RAIG-Records/03-Interests/Projects/DiscreteX/examples/two_sat_solver.cpp) | 2-CNF formula translation to directed implication graphs and linear-time Aspvall-Plass-Tarjan SCC solver. |

---

## 7. How to Build and Run Each Example

All standalone examples are located in the `examples/` directory and can be compiled individually or in batch.

### Running via Make (Batch)
```bash
make examples
```

### Compiling and Running Individually

#### 1. Shortest Paths (`examples/shortest_paths.cpp`)
```bash
g++ -std=c++20 -O2 -Iinclude examples/shortest_paths.cpp -o shortest_paths
./shortest_paths
```
*Expected Output Signature:*
```text
=== DiscreteX Example: Weighted Shortest Paths and Reconstruction ===
1. Weighted graph constructed: |V| = 6, |E| = 7
2. Dijkstra distances from source 0:
     to vertex 0: dist = 0
     to vertex 1: dist = 4
     to vertex 2: dist = 2
     to vertex 3: dist = 9
     to vertex 4: dist = 11
     to vertex 5: UNREACHABLE
3. Reconstructed shortest path to vertex 4: 0 -> 1 -> 3 -> 4
   Path integrity verification: VERIFIED
```

#### 2. Network Flow and Min-Cut (`examples/network_flow_min_cut.cpp`)
```bash
g++ -std=c++20 -O2 -Iinclude examples/network_flow_min_cut.cpp -o network_flow_min_cut
./network_flow_min_cut
```
*Expected Output Signature:*
```text
=== DiscreteX Example: Maximum Flow and Minimum Cut Duality ===
1. Flow network constructed: |V| = 6, |E| = 10
2. Dinic algorithm completed: Maximum Flow = 23
3. Flow conservation at all intermediate vertices: VERIFIED
4. Minimum Cut Capacity = 23
   Max-Flow == Min-Cut Duality: CERTIFIED
```

#### 3. Quotient Group Isomorphism (`examples/quotient_group_isomorphism.cpp`)
```bash
g++ -std=c++20 -O2 -Iinclude examples/quotient_group_isomorphism.cpp -o quotient_group_isomorphism
./quotient_group_isomorphism
```
*Expected Output Signature:*
```text
=== DiscreteX Example: Normal Subgroups and First Isomorphism Theorem ===
1. Group S_3 constructed (order = 6, abelian = false)
2. Subgroup A_3 = {0, 1, 2} normal in S_3: YES
   Subgroup H = {0, 3} (generated by transposition) normal in S_3: NO
3. Quotient Group S_3 / A_3 constructed with order 2
4. First Isomorphism Theorem S_3 / A_3 =~ Z_2: CERTIFIED
```

#### 4. Regex to Minimized DFA Pipeline (`examples/regex_to_min_dfa.cpp`)
```bash
g++ -std=c++20 -O2 -Iinclude examples/regex_to_min_dfa.cpp -o regex_to_min_dfa
./regex_to_min_dfa
```
*Expected Output Signature:*
```text
=== DiscreteX Example: Regex to Minimized DFA Pipeline ===
1. Regular Expression AST: ((((0 | 1))* . 0) . 1)
2. Thompson NFA constructed with 12 states.
3. Powerset DFA constructed with 4 states.
4. Minimal DFA constructed with 3 canonical states.
5. Word Evaluations:
     Word "01" -> ACCEPTED
     Word "1101" -> ACCEPTED
     Word "10" -> REJECTED
6. Formal Language Identities:
     L((0|1)*) is universal: YES
     (0|1)* equivalent to (0* 1*)*: CERTIFIED
```

#### 5. 2-SAT Implication Solver (`examples/two_sat_solver.cpp`)
```bash
g++ -std=c++20 -O2 -Iinclude examples/two_sat_solver.cpp -o two_sat_solver
./two_sat_solver
```
*Expected Output Signature:*
```text
=== DiscreteX Example: 2-SAT Satisfiability via Implication Graphs ===
1. 2-CNF Formula constructed with 3 variables and 4 clauses.
2. Implication graph constructed: |V| = 6 (2 literals per variable), |E| = 8
3. 2-SAT Satisfiability: SATISFIABLE
   Extracted Truth Assignment (Model):
     x0 = true
     x1 = false
     x2 = false
   Clause Satisfaction Check: ALL SATISFIED
```

---

## 8. Recommended Reading Order

To build a complete mental model without unnecessary cognitive overhead, follow this progression:

```
1. README.md                      (High-level overview & installation)
   │
2. docs/start_here.md             (This orientation, sandbox, and track selector)
   │
3. examples/                      (Run and inspect the 5 standalone programs)
   │
4. docs/api_index.md              (Compact quick-lookup reference for all 34 headers)
   │
5. docs/theory_to_code_tour.md    (Step-by-step mathematical pipeline walkthroughs)
   │
6. docs/architecture.md           (Deep systems dive, hardware acceleration, memory models)
```
