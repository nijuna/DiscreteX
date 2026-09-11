#include <iostream>
#include <vector>
#include <discretex/discretex.hpp>

int main() {
    std::cout << "=== DiscreteX Example: Regex to Minimized DFA Pipeline ===\n";

    using namespace discretex::automata;

    // 1. Construct regular expression: (0 | 1)* . 0 . 1
    // Recognizes binary strings ending in "01"
    auto r0 = literal(0);
    auto r1 = literal(1);
    auto regex_suffix01 = star(r0 | r1) + r0 + r1;

    std::cout << "1. Regular Expression AST: " << to_string(regex_suffix01) << "\n";

    // 2. Thompson Inductive Construction (Regex -> epsilon-NFA)
    const std::size_t alphabet_size = 2; // {0, 1}
    auto nfa_suffix = to_nfa(regex_suffix01, alphabet_size);
    std::cout << "2. Thompson NFA constructed with " << nfa_suffix.state_count() << " states.\n";

    // 3. Powerset Construction (NFA -> total DFA)
    auto sc_res = subset_construction(nfa_suffix);
    std::cout << "3. Powerset DFA constructed with " << sc_res.machine.state_count() << " states.\n";

    // 4. Hopcroft Partition Refinement Minimization (DFA -> DFA_min)
    auto min_res = minimize_dfa_with_partition(sc_res.machine);
    const auto& min_dfa = min_res.machine;
    std::cout << "4. Minimal DFA constructed with " << min_dfa.state_count() << " canonical states.\n";

    std::cout << "   Canonical Nerode Equivalence Classes:\n";
    for (std::size_t c = 0; c < min_res.partition_blocks.size(); ++c) {
        std::cout << "     Block " << c << " (accepting=" << (min_dfa.is_accepting(c) ? "true" : "false") << "): { ";
        for (std::size_t q : min_res.partition_blocks[c]) {
            std::cout << q << " ";
        }
        std::cout << "}\n";
    }

    // 5. Test Word Acceptance
    std::vector<std::vector<std::size_t>> test_words = {
        {0, 1},          // "01" (accepted)
        {1, 1, 0, 1},    // "1101" (accepted)
        {0, 0, 1},       // "001" (accepted)
        {1, 0},          // "10" (rejected)
        {0, 1, 0},       // "010" (rejected)
        {}               // "" (rejected)
    };

    std::cout << "5. Word Evaluations:\n";
    for (const auto& w : test_words) {
        bool acc = min_dfa.accepts(w);
        std::cout << "     Word \"";
        for (std::size_t bit : w) std::cout << bit;
        std::cout << "\" -> " << (acc ? "ACCEPTED" : "REJECTED") << "\n";
    }

    // 6. Language Decision Procedures
    auto star_01 = star(r0 | r1);
    auto star_star = star(star(r0) + star(r1));
    auto dfa_univ1 = to_dfa(star_01, alphabet_size);
    auto dfa_univ2 = to_dfa(star_star, alphabet_size);

    std::cout << "6. Formal Language Identities:\n";
    std::cout << "     L((0|1)*) is universal: " << (is_universal_language(dfa_univ1) ? "YES" : "NO") << "\n";
    std::cout << "     L((0|1)* 0 1) is empty: " << (is_empty_language(min_dfa) ? "YES" : "NO") << "\n";
    std::cout << "     L(0 1) subset L((0|1)* 0 1): " << (is_language_included(to_dfa(r0 + r1, alphabet_size), min_dfa) ? "YES" : "NO") << "\n";
    std::cout << "     (0|1)* equivalent to (0* 1*)*: " << (is_language_equivalent(dfa_univ1, dfa_univ2) ? "CERTIFIED" : "FAILED") << "\n";

    return 0;
}
