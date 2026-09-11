#pragma once

// Core
#include "core/domain.hpp"
#include "core/relation_concepts.hpp"
#include "core/dsu.hpp"

// Storage
#include "storage/bit_matrix.hpp"

// Relations & Graphs
#include "relation/dense_relation.hpp"
#include "relation/equivalence.hpp"
#include "graph/sparse_graph.hpp"
#include "graph/bipartite_graph.hpp"
#include "graph/weighted_graph.hpp"
#include "graph/flow_network.hpp"

// Views
#include "views/transpose_view.hpp"

// Algorithms
#include "algorithms/bfs.hpp"
#include "algorithms/topological_sort.hpp"
#include "algorithms/tarjan_scc.hpp"
#include "algorithms/condensation.hpp"
#include "algorithms/bipartite_matching.hpp"
#include "algorithms/minimum_spanning_tree.hpp"
#include "algorithms/network_flow.hpp"

// Combinatorics
#include "combinatorics/counting.hpp"
#include "combinatorics/k_subsets.hpp"
#include "combinatorics/power_set.hpp"
#include "combinatorics/permutations.hpp"
#include "combinatorics/integer_partitions.hpp"
#include "combinatorics/set_partitions.hpp"
#include "combinatorics/projection.hpp"

// Order Theory
#include "order/poset.hpp"

// Logic
#include "logic/formula.hpp"
#include "logic/truth_table.hpp"
#include "logic/normal_forms.hpp"
#include "logic/two_sat.hpp"

// Number Theory
#include "number_theory/euclidean.hpp"
#include "number_theory/modular.hpp"
#include "number_theory/primes.hpp"
#include "number_theory/congruence.hpp"
