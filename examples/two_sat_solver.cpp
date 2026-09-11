#include <iostream>
#include <vector>
#include <discretex/discretex.hpp>

int main() {
    std::cout << "=== DiscreteX Example: 2-SAT Satisfiability via Implication Graphs ===\n";

    using namespace discretex::logic;

    // Constraint Satisfaction Problem:
    // 3 variables: x0, x1, x2
    // Clauses:
    // (x0 v x1) ^ (~x1 v x2) ^ (~x2 v ~x0) ^ (x0 v ~x2)
    formula_2cnf f(3);
    f.add_clause(pos(0), pos(1));
    f.add_clause(neg(1), pos(2));
    f.add_clause(neg(2), neg(0));
    f.add_clause(pos(0), neg(2));

    std::cout << "1. 2-CNF Formula constructed with " << f.variable_count() 
              << " variables and " << f.clauses().size() << " clauses.\n";

    // 2. Build Directed Implication Graph
    auto imp_graph = implication_graph(f);
    std::cout << "2. Implication graph constructed: |V| = " << imp_graph.domain_size() 
              << " (2 literals per variable), |E| = " << imp_graph.edge_count() << "\n";

    // 3. Solve 2-SAT via Aspvall-Plass-Tarjan Linear SCC Engine
    auto res = solve_2sat(f);
    std::cout << "3. 2-SAT Satisfiability: " << (res.satisfiable ? "SATISFIABLE" : "UNSATISFIABLE") << "\n";

    if (res.satisfiable) {
        std::cout << "   Extracted Truth Assignment (Model):\n";
        for (std::size_t i = 0; i < res.assignment.size(); ++i) {
            std::cout << "     x" << i << " = " << (res.assignment[i] ? "true" : "false") << "\n";
        }

        // Verify assignment against all clauses
        bool all_satisfied = true;
        for (const auto& cl : f.clauses()) {
            bool left = res.assignment[cl.left.var] ^ cl.left.negated;
            bool right = res.assignment[cl.right.var] ^ cl.right.negated;
            if (!left && !right) {
                all_satisfied = false;
                break;
            }
        }
        std::cout << "   Clause Satisfaction Check: " << (all_satisfied ? "ALL SATISFIED" : "ERROR") << "\n";
    }

    return 0;
}
