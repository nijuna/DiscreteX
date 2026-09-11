# DiscreteX

DiscreteX is a modern, unified C++20 library for discrete mathematics and discrete structures.

In the standard C++ ecosystem, discrete mathematics is fragmented: graph libraries, combinatorics utilities, logic engines, and number-theoretic algorithms often use conflicting abstractions, bespoke container types, and inconsistent indexing models. 

The goal of DiscreteX is to unify these domains into a mathematically coherent, high-performance C++20 library. In DiscreteX, structures are not isolated: partially ordered sets naturally produce directed acyclic graphs; propositional valuation spaces are isomorphic to Boolean lattices; integer partitions and set partitions connect directly with equivalence relations; and divisibility relations on integers form distributive lattices that share the exact same order-theoretic and graph algorithms.

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

### 1. Core Relations & Equivalence Structures (`discretex/core`, `discretex/relation`)
- Mathematical binary relations over finite domains with packed 64-bit word storage and bit-scan fiber traversal.
- Equivalence relation axiom verification (`is_equivalence_relation`) requiring only `concepts::Relation`.
- Canonical quotient set extraction (`equivalence_classes_rgs`, `equivalence_classes`, `quotient_size`).
- Two-way bridge between set partitions and dense equivalence relations (`relation_from_partition`).
- In-place Warshall transitive closure.
- Generic Breadth-First Search (`algorithms::bfs`) operating uniformly across dense relations, sparse graphs, and transpose views.

### 2. Enumerative Combinatorics & Generators (`discretex/combinatorics`)
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

### 3. Order Theory & Lattices (`discretex/order`)
- Closure-backed partially ordered sets (`poset`).
- Exact covering relation reduction: $C = S \\setminus (S \\circ S)$.
- Hasse diagram construction (`poset::hasse_diagram`).
- Topological linear extensions (`poset::linear_extension`) via Kahn's algorithm.
- Lattice testing (`poset::is_lattice`), meets, and joins.
- Strict constructor validation (`from_dag_closure` vs. `from_hasse_diagram` rejecting transitive shortcuts).

### 4. Propositional Logic & Valuation Spaces (`discretex/logic`)
- Pure AST formula representation (`formula`) supporting variables, negation, conjunction, disjunction, implication, and biconditional equivalence.
- Bitmask formula evaluation over 64-bit assignments ($O(1)$ variable lookup).
- Truth table generator (`truth_table`) with tautology, contradiction, and satisfiability checking.
- Semantic equivalence (`equivalent`) and logical entailment (`entails`).
- Normal forms:
  - Negation Normal Form (`to_nnf`) via De Morgan's laws and double-negation elimination.
  - Algebraic DNF & CNF (`to_dnf`, `to_cnf`) via distributive laws.
  - Canonical DNF & CNF (`to_canonical_dnf`, `to_canonical_cnf`) via minterm and maxterm expansions.
- **Valuation Space Bridge**: Direct isomorphism between satisfying valuation subsets and Boolean lattice operations ($\\cap \\leftrightarrow \\land$, $\\cup \\leftrightarrow \\lor$, $\\setminus \\leftrightarrow \\neg$, $\\subseteq \\leftrightarrow \\models$).

### 5. Number Theory & Modular Arithmetic (`discretex/number_theory`)
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

---

## Build and Test

DiscreteX requires a C++20 compliant compiler (GCC 11+, Clang 13+).

```bash
# Compile and run the complete test suite
make test

# Clean artifacts
make clean
```

All algorithms and bridges are verified with 100% test coverage under `-std=c++20 -Wall -Wextra -Wpedantic`.

---

## License

MIT License.
