#pragma once

#include "dfa.hpp"
#include "nfa.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>
#include <span>
#include <queue>
#include <map>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <bit>
#include <ranges>
#include <concepts>
#include <initializer_list>
#include <utility>

namespace discretex::automata {

/**
 * @brief Dense bitset-style state subset representation for automata algorithms.
 */
class state_set {
public:
    state_set() = default;

    explicit state_set(std::size_t universe_size)
        : universe_size_(universe_size),
          words_((universe_size + 63) / 64, 0ULL) {}

    [[nodiscard]] std::size_t universe_size() const noexcept { return universe_size_; }

    void insert(std::size_t q) noexcept {
        if (q < universe_size_) {
            words_[q / 64] |= (1ULL << (q % 64));
        }
    }

    [[nodiscard]] bool contains(std::size_t q) const noexcept {
        if (q >= universe_size_) return false;
        return (words_[q / 64] & (1ULL << (q % 64))) != 0ULL;
    }

    void union_with(const state_set& other) noexcept {
        for (std::size_t i = 0; i < words_.size(); ++i) {
            words_[i] |= other.words_[i];
        }
    }

    [[nodiscard]] bool empty() const noexcept {
        for (auto w : words_) {
            if (w != 0ULL) return false;
        }
        return true;
    }

    [[nodiscard]] std::vector<std::size_t> to_vector() const {
        std::vector<std::size_t> res;
        for (std::size_t i = 0; i < words_.size(); ++i) {
            uint64_t w = words_[i];
            while (w != 0ULL) {
                int tz = std::countr_zero(w);
                res.push_back(i * 64 + static_cast<std::size_t>(tz));
                w &= (w - 1ULL);
            }
        }
        return res;
    }

    [[nodiscard]] bool operator==(const state_set& other) const noexcept = default;
    [[nodiscard]] auto operator<=>(const state_set& other) const noexcept = default;

private:
    std::size_t universe_size_{0};
    std::vector<uint64_t> words_;
};

/**
 * @brief Compute the \(\varepsilon\)-closure of a single NFA state as a dense state set.
 */
inline state_set epsilon_closure_set(const nfa& m, std::size_t state) {
    state_set closure(m.state_count());
    if (state >= m.state_count()) return closure;

    std::vector<std::size_t> queue;
    closure.insert(state);
    queue.push_back(state);

    std::size_t head = 0;
    while (head < queue.size()) {
        std::size_t u = queue[head++];
        for (std::size_t v : m.epsilon_transitions(u)) {
            if (!closure.contains(v)) {
                closure.insert(v);
                queue.push_back(v);
            }
        }
    }
    return closure;
}

/**
 * @brief Compute the \(\varepsilon\)-closure of a single NFA state as a sorted vector.
 */
inline std::vector<std::size_t> epsilon_closure(const nfa& m, std::size_t state) {
    return epsilon_closure_set(m, state).to_vector();
}

/**
 * @brief Compute the \(\varepsilon\)-closure of a collection of NFA states as a dense state set.
 */
template <typename Range>
    requires std::ranges::input_range<Range> &&
             std::convertible_to<std::ranges::range_value_t<Range>, std::size_t>
inline state_set epsilon_closure_set(const nfa& m, const Range& states) {
    state_set closure(m.state_count());
    std::vector<std::size_t> queue;
    for (auto s : states) {
        std::size_t u = static_cast<std::size_t>(s);
        if (u < m.state_count() && !closure.contains(u)) {
            closure.insert(u);
            queue.push_back(u);
        }
    }

    std::size_t head = 0;
    while (head < queue.size()) {
        std::size_t u = queue[head++];
        for (std::size_t v : m.epsilon_transitions(u)) {
            if (!closure.contains(v)) {
                closure.insert(v);
                queue.push_back(v);
            }
        }
    }
    return closure;
}

/**
 * @brief Compute the \(\varepsilon\)-closure of a collection of NFA states as a sorted vector.
 */
template <typename Range>
    requires std::ranges::input_range<Range> &&
             std::convertible_to<std::ranges::range_value_t<Range>, std::size_t>
inline std::vector<std::size_t> epsilon_closure(const nfa& m, const Range& states) {
    return epsilon_closure_set(m, states).to_vector();
}

/**
 * @brief Test whether a word of symbol indices is accepted by an NFA.
 */
template <typename Range>
    requires std::ranges::input_range<Range> &&
             std::convertible_to<std::ranges::range_value_t<Range>, std::size_t>
inline bool accepts(const nfa& m, const Range& word) {
    if (m.state_count() == 0) return false;

    state_set current = epsilon_closure_set(m, m.start_state());
    for (auto sym : word) {
        std::size_t a = static_cast<std::size_t>(sym);
        if (a >= m.alphabet_size()) return false;

        state_set next_set(m.state_count());
        auto curr_states = current.to_vector();
        for (std::size_t u : curr_states) {
            for (std::size_t v : m.transitions(u, a)) {
                next_set.union_with(epsilon_closure_set(m, v));
            }
        }
        current = std::move(next_set);
    }

    for (std::size_t u : current.to_vector()) {
        if (m.is_accepting(u)) return true;
    }
    return false;
}

inline bool accepts(const nfa& m, std::initializer_list<std::size_t> word) {
    return accepts<std::initializer_list<std::size_t>>(m, word);
}

/**
 * @brief Result of subset construction (determinization).
 */
struct subset_construction_result {
    dfa machine;
    std::vector<std::vector<std::size_t>> dfa_state_to_nfa_subset;
};

/**
 * @brief Convert an NFA to an equivalent DFA via the powerset construction.
 */
inline subset_construction_result subset_construction(const nfa& m) {
    std::size_t sigma = m.alphabet_size();
    if (m.state_count() == 0) {
        return subset_construction_result{dfa(0, sigma, 0), {}};
    }

    state_set start_subset = epsilon_closure_set(m, m.start_state());
    std::map<state_set, std::size_t> subset_to_id;
    std::vector<state_set> discovered;
    std::queue<std::size_t> worklist;

    subset_to_id[start_subset] = 0;
    discovered.push_back(start_subset);
    worklist.push(0);

    std::vector<std::vector<std::size_t>> dfa_transitions;

    while (!worklist.empty()) {
        std::size_t u = worklist.front();
        worklist.pop();

        dfa_transitions.emplace_back(sigma, 0);
        const state_set& curr_set = discovered[u];
        auto u_states = curr_set.to_vector();

        for (std::size_t a = 0; a < sigma; ++a) {
            state_set next_set(m.state_count());
            for (std::size_t q : u_states) {
                for (std::size_t r : m.transitions(q, a)) {
                    next_set.union_with(epsilon_closure_set(m, r));
                }
            }

            auto it = subset_to_id.find(next_set);
            std::size_t v = 0;
            if (it == subset_to_id.end()) {
                v = discovered.size();
                subset_to_id[next_set] = v;
                discovered.push_back(next_set);
                worklist.push(v);
            } else {
                v = it->second;
            }
            dfa_transitions[u][a] = v;
        }
    }

    std::size_t num_dfa_states = discovered.size();
    dfa out_dfa(num_dfa_states, sigma, 0);
    std::vector<std::vector<std::size_t>> mapping(num_dfa_states);

    for (std::size_t u = 0; u < num_dfa_states; ++u) {
        mapping[u] = discovered[u].to_vector();
        for (std::size_t a = 0; a < sigma; ++a) {
            out_dfa.set_transition(u, a, dfa_transitions[u][a]);
        }
        for (std::size_t q : mapping[u]) {
            if (m.is_accepting(q)) {
                out_dfa.set_accepting(u, true);
                break;
            }
        }
    }

    return subset_construction_result{std::move(out_dfa), std::move(mapping)};
}

/**
 * @brief Determinize an NFA into an equivalent DFA.
 */
inline dfa determinize(const nfa& m) {
    return subset_construction(m).machine;
}

/**
 * @brief Complement a total DFA: recognizes \(\Sigma^* \setminus L(D)\).
 */
inline dfa complement(const dfa& d) {
    dfa res = d;
    for (std::size_t q = 0; q < res.state_count(); ++q) {
        res.set_accepting(q, !d.is_accepting(q));
    }
    return res;
}

/**
 * @brief Eliminate unreachable states from a DFA, renumbering reachable states to [0, k).
 */
inline dfa trim_unreachable(const dfa& d) {
    if (d.state_count() == 0) return d;

    std::vector<bool> visited(d.state_count(), false);
    std::vector<std::size_t> queue;
    visited[d.start_state()] = true;
    queue.push_back(d.start_state());

    std::size_t head = 0;
    while (head < queue.size()) {
        std::size_t u = queue[head++];
        for (std::size_t a = 0; a < d.alphabet_size(); ++a) {
            std::size_t v = d.transition(u, a);
            if (!visited[v]) {
                visited[v] = true;
                queue.push_back(v);
            }
        }
    }

    if (queue.size() == d.state_count()) {
        return d;
    }

    std::vector<std::size_t> old_to_new(d.state_count(), std::numeric_limits<std::size_t>::max());
    std::size_t new_count = 0;
    for (std::size_t q = 0; q < d.state_count(); ++q) {
        if (visited[q]) {
            old_to_new[q] = new_count++;
        }
    }

    dfa trimmed(new_count, d.alphabet_size(), old_to_new[d.start_state()]);
    for (std::size_t old_q = 0; old_q < d.state_count(); ++old_q) {
        if (!visited[old_q]) continue;
        std::size_t new_q = old_to_new[old_q];
        if (d.is_accepting(old_q)) {
            trimmed.set_accepting(new_q, true);
        }
        for (std::size_t a = 0; a < d.alphabet_size(); ++a) {
            trimmed.set_transition(new_q, a, old_to_new[d.transition(old_q, a)]);
        }
    }
    return trimmed;
}

/**
 * @brief Synchronous direct product intersection of two DFAs: recognizes \(L(D_1) \cap L(D_2)\).
 */
inline dfa intersect(const dfa& d1, const dfa& d2, bool trim = true) {
    if (d1.alphabet_size() != d2.alphabet_size()) {
        throw std::invalid_argument("DFA intersection requires identical alphabet sizes.");
    }
    std::size_t n1 = d1.state_count();
    std::size_t n2 = d2.state_count();
    std::size_t sigma = d1.alphabet_size();
    if (n1 == 0 || n2 == 0) {
        return dfa(0, sigma, 0);
    }

    std::size_t total_states = n1 * n2;
    std::size_t start = d1.start_state() * n2 + d2.start_state();
    dfa prod(total_states, sigma, start);

    for (std::size_t p = 0; p < n1; ++p) {
        for (std::size_t q = 0; q < n2; ++q) {
            std::size_t prod_state = p * n2 + q;
            if (d1.is_accepting(p) && d2.is_accepting(q)) {
                prod.set_accepting(prod_state, true);
            }
            for (std::size_t a = 0; a < sigma; ++a) {
                std::size_t next_p = d1.transition(p, a);
                std::size_t next_q = d2.transition(q, a);
                std::size_t next_prod = next_p * n2 + next_q;
                prod.set_transition(prod_state, a, next_prod);
            }
        }
    }

    if (trim) {
        return trim_unreachable(prod);
    }
    return prod;
}

/**
 * @brief Structured result of DFA minimization with quotient equivalence partition.
 */
struct minimization_result {
    dfa machine;
    std::vector<std::vector<std::size_t>> partition_blocks;
    std::vector<std::size_t> state_to_block;
};

/**
 * @brief Hopcroft partition refinement DFA minimization.
 *
 * Produces the canonical minimal quotient DFA and Nerode equivalence classes.
 */
inline minimization_result minimize_dfa_with_partition(const dfa& d) {
    dfa d_trim = trim_unreachable(d);
    std::size_t N = d_trim.state_count();
    std::size_t sigma = d_trim.alphabet_size();

    if (N == 0) {
        return minimization_result{dfa(0, sigma, 0), {}, {}};
    }
    if (N == 1) {
        return minimization_result{d_trim, {{0}}, {0}};
    }

    // Split states into accepting F and non-accepting NF
    std::vector<std::size_t> F;
    std::vector<std::size_t> NF;
    for (std::size_t q = 0; q < N; ++q) {
        if (d_trim.is_accepting(q)) {
            F.push_back(q);
        } else {
            NF.push_back(q);
        }
    }

    // Degenerate cases: all accepting or all non-accepting
    if (F.empty() || NF.empty()) {
        bool all_acc = !F.empty();
        dfa min_m(1, sigma, 0);
        min_m.set_accepting(0, all_acc);
        for (std::size_t a = 0; a < sigma; ++a) {
            min_m.set_transition(0, a, 0);
        }
        std::vector<std::size_t> all_states(N);
        std::iota(all_states.begin(), all_states.end(), 0);
        return minimization_result{std::move(min_m), {all_states}, std::vector<std::size_t>(N, 0)};
    }

    // Initial partition: block 0 = F, block 1 = NF
    std::vector<std::vector<std::size_t>> P;
    P.push_back(F);
    P.push_back(NF);

    std::vector<std::size_t> block_of(N);
    for (std::size_t q : F) block_of[q] = 0;
    for (std::size_t q : NF) block_of[q] = 1;

    // Hopcroft worklist: add the smaller of F and NF
    std::vector<bool> in_w(2, false);
    std::vector<std::size_t> W;
    if (F.size() <= NF.size()) {
        W.push_back(0);
        in_w[0] = true;
    } else {
        W.push_back(1);
        in_w[1] = true;
    }

    // Precalculate inverse transitions: inv_delta[a][v] = {u | delta(u, a) == v}
    std::vector<std::vector<std::vector<std::size_t>>> inv_delta(
        sigma, std::vector<std::vector<std::size_t>>(N)
    );
    for (std::size_t u = 0; u < N; ++u) {
        for (std::size_t a = 0; a < sigma; ++a) {
            std::size_t v = d_trim.transition(u, a);
            inv_delta[a][v].push_back(u);
        }
    }

    std::vector<std::size_t> count_in_X(N + 2, 0);
    std::vector<bool> in_X(N, false);

    while (!W.empty()) {
        std::size_t B = W.back();
        W.pop_back();
        in_w[B] = false;

        for (std::size_t c = 0; c < sigma; ++c) {
            std::vector<std::size_t> X;
            for (std::size_t v : P[B]) {
                for (std::size_t u : inv_delta[c][v]) {
                    if (!in_X[u]) {
                        in_X[u] = true;
                        X.push_back(u);
                    }
                }
            }

            std::vector<std::size_t> touched_blocks;
            for (std::size_t u : X) {
                std::size_t b = block_of[u];
                if (b >= count_in_X.size()) {
                    count_in_X.resize(b + 1, 0);
                }
                if (count_in_X[b] == 0) {
                    touched_blocks.push_back(b);
                }
                count_in_X[b]++;
            }

            for (std::size_t j : touched_blocks) {
                if (count_in_X[j] < P[j].size()) {
                    std::vector<std::size_t> Y1;
                    std::vector<std::size_t> Y2;
                    Y1.reserve(count_in_X[j]);
                    Y2.reserve(P[j].size() - count_in_X[j]);

                    for (std::size_t u : P[j]) {
                        if (in_X[u]) {
                            Y1.push_back(u);
                        } else {
                            Y2.push_back(u);
                        }
                    }

                    std::size_t new_id = P.size();
                    P[j] = std::move(Y1);
                    P.push_back(std::move(Y2));
                    in_w.push_back(false);
                    if (new_id >= count_in_X.size()) {
                        count_in_X.resize(new_id + 1, 0);
                    }

                    for (std::size_t u : P[new_id]) {
                        block_of[u] = new_id;
                    }

                    if (in_w[j]) {
                        in_w[new_id] = true;
                        W.push_back(new_id);
                    } else {
                        if (P[j].size() <= P[new_id].size()) {
                            in_w[j] = true;
                            W.push_back(j);
                        } else {
                            in_w[new_id] = true;
                            W.push_back(new_id);
                        }
                    }
                }
                count_in_X[j] = 0;
            }

            for (std::size_t u : X) {
                in_X[u] = false;
            }
        }
    }

    // Sort states within each block
    for (auto& block : P) {
        std::sort(block.begin(), block.end());
    }

    // Canonical block ordering:
    // Block containing start state comes first (state 0), remaining blocks sorted by minimum element
    std::size_t start_block = block_of[d_trim.start_state()];
    std::vector<std::size_t> block_order(P.size());
    std::iota(block_order.begin(), block_order.end(), 0);
    std::sort(block_order.begin(), block_order.end(), [&](std::size_t i, std::size_t j) {
        if (i == start_block) return true;
        if (j == start_block) return false;
        return P[i][0] < P[j][0];
    });

    std::vector<std::size_t> old_to_canonical(P.size());
    std::vector<std::vector<std::size_t>> canonical_blocks(P.size());
    for (std::size_t c = 0; c < P.size(); ++c) {
        std::size_t old_b = block_order[c];
        old_to_canonical[old_b] = c;
        canonical_blocks[c] = P[old_b];
    }

    std::vector<std::size_t> state_to_canonical(N);
    for (std::size_t q = 0; q < N; ++q) {
        state_to_canonical[q] = old_to_canonical[block_of[q]];
    }

    // Build canonical quotient DFA
    std::size_t num_blocks = P.size();
    dfa min_dfa(num_blocks, sigma, 0);

    for (std::size_t c = 0; c < num_blocks; ++c) {
        std::size_t rep = canonical_blocks[c][0];
        min_dfa.set_accepting(c, d_trim.is_accepting(rep));
        for (std::size_t a = 0; a < sigma; ++a) {
            std::size_t target_state = d_trim.transition(rep, a);
            std::size_t target_c = state_to_canonical[target_state];
            min_dfa.set_transition(c, a, target_c);
        }
    }

    return minimization_result{
        std::move(min_dfa),
        std::move(canonical_blocks),
        std::move(state_to_canonical)
    };
}

/**
 * @brief Minimize a DFA using Hopcroft partition refinement.
 */
inline dfa minimize_dfa(const dfa& d) {
    return minimize_dfa_with_partition(d).machine;
}

/**
 * @brief Determine whether two DFAs accept the same language via product exploration.
 */
inline bool language_equivalent(const dfa& d1, const dfa& d2) {
    if (d1.alphabet_size() != d2.alphabet_size()) {
        return false;
    }
    std::size_t n1 = d1.state_count();
    std::size_t n2 = d2.state_count();
    std::size_t sigma = d1.alphabet_size();
    if (n1 == 0 && n2 == 0) return true;
    if (n1 == 0 || n2 == 0) {
        const dfa& non_empty = (n1 > 0) ? d1 : d2;
        dfa t = trim_unreachable(non_empty);
        for (std::size_t q = 0; q < t.state_count(); ++q) {
            if (t.is_accepting(q)) return false;
        }
        return true;
    }

    std::vector<bool> visited(n1 * n2, false);
    std::vector<std::pair<std::size_t, std::size_t>> queue;

    std::size_t s1 = d1.start_state();
    std::size_t s2 = d2.start_state();
    visited[s1 * n2 + s2] = true;
    queue.emplace_back(s1, s2);

    std::size_t head = 0;
    while (head < queue.size()) {
        auto [p, q] = queue[head++];
        if (d1.is_accepting(p) != d2.is_accepting(q)) {
            return false;
        }
        for (std::size_t a = 0; a < sigma; ++a) {
            std::size_t np = d1.transition(p, a);
            std::size_t nq = d2.transition(q, a);
            std::size_t n_idx = np * n2 + nq;
            if (!visited[n_idx]) {
                visited[n_idx] = true;
                queue.emplace_back(np, nq);
            }
        }
    }
    return true;
}

} // namespace discretex::automata
