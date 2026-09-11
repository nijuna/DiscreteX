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

All execution runs over dense, cache-friendly integer indices (`index_domain`), while arbitrary user entities (strings, coordinates, custom objects) are mapped once at the boundary via `mapped_domain<T>`.

---

## 2. Fast Setup (Zero External Dependencies)

DiscreteX requires an ISO C++20 compliant compiler (GCC 11+, Clang 13+, or MSVC 19.29+).

### Option A: Header-Only Direct Include (Fastest for experimentation)
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

## 3. Your First 10-Line Sandbox Program

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

## 4. Choose Your Track

Depending on your mathematical or computational objective, explore the following curated pathways:

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

## 5. Recommended Reading Order

To build a complete mental model without unnecessary cognitive overhead, follow this progression:

```
1. README.md                      (High-level overview & installation)
   │
2. docs/start_here.md             (This orientation & 5-minute sandbox)
   │
3. examples/                      (Run and inspect the 5 standalone programs)
   │
4. docs/api_index.md              (Compact quick-lookup reference for all 34 headers)
   │
5. docs/theory_to_code_tour.md    (Step-by-step mathematical pipeline walkthroughs)
   │
6. docs/architecture.md           (Deep systems dive, hardware acceleration, memory models)
```
