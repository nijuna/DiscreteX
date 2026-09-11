#pragma once

#include <cstddef>
#include <vector>
#include <span>
#include <stdexcept>
#include <algorithm>

namespace discretex::automata {

/**
 * @brief Nondeterministic Finite Automaton (NFA) with \(\varepsilon\)-transitions.
 *
 * Implements a finite state transition system where:
 * - States are indexed in [0, state_count).
 * - Alphabet symbols are indexed in [0, alphabet_size).
 * - Transitions are set-valued per (state, symbol).
 * - \(\varepsilon\)-transitions (spontaneous transitions consuming no input symbol)
 *   are stored separately from alphabet transitions.
 */
class nfa {
public:
    nfa() = default;

    /**
     * @brief Construct an NFA with specified state count and alphabet size.
     * @param state_count Number of states |Q|.
     * @param alphabet_size Cardinality of alphabet |\Sigma|.
     * @param start_state Initial state q_0 in [0, state_count).
     */
    nfa(std::size_t state_count, std::size_t alphabet_size, std::size_t start_state = 0)
        : state_count_(state_count),
          alphabet_size_(alphabet_size),
          start_state_(start_state),
          transitions_(state_count * alphabet_size),
          epsilon_transitions_(state_count),
          accepting_(state_count, false) {
        if (state_count > 0 && start_state >= state_count) {
            throw std::invalid_argument("NFA start state index out of bounds.");
        }
    }

    [[nodiscard]] std::size_t state_count() const noexcept { return state_count_; }
    [[nodiscard]] std::size_t alphabet_size() const noexcept { return alphabet_size_; }
    [[nodiscard]] std::size_t start_state() const noexcept { return start_state_; }

    void set_start_state(std::size_t q) {
        if (q >= state_count_) {
            throw std::invalid_argument("NFA start state index out of bounds.");
        }
        start_state_ = q;
    }

    [[nodiscard]] bool is_accepting(std::size_t q) const {
        if (q >= state_count_) {
            throw std::out_of_range("NFA state index out of bounds.");
        }
        return accepting_[q];
    }

    void set_accepting(std::size_t q, bool val = true) {
        if (q >= state_count_) {
            throw std::out_of_range("NFA state index out of bounds.");
        }
        accepting_[q] = val;
    }

    [[nodiscard]] std::vector<std::size_t> accepting_states() const {
        std::vector<std::size_t> res;
        for (std::size_t q = 0; q < state_count_; ++q) {
            if (accepting_[q]) {
                res.push_back(q);
            }
        }
        return res;
    }

    /**
     * @brief Add a directed transition on alphabet symbol: delta(from, symbol) contains to.
     */
    void add_transition(std::size_t from, std::size_t symbol, std::size_t to) {
        if (from >= state_count_ || to >= state_count_) {
            throw std::out_of_range("NFA transition state index out of bounds.");
        }
        if (symbol >= alphabet_size_) {
            throw std::out_of_range("NFA transition symbol index out of bounds.");
        }
        auto& dests = transitions_[from * alphabet_size_ + symbol];
        if (std::find(dests.begin(), dests.end(), to) == dests.end()) {
            dests.push_back(to);
            std::sort(dests.begin(), dests.end());
        }
    }

    /**
     * @brief Add a directed \(\varepsilon\)-transition: delta(from, \varepsilon) contains to.
     */
    void add_epsilon_transition(std::size_t from, std::size_t to) {
        if (from >= state_count_ || to >= state_count_) {
            throw std::out_of_range("NFA epsilon transition state index out of bounds.");
        }
        auto& dests = epsilon_transitions_[from];
        if (std::find(dests.begin(), dests.end(), to) == dests.end()) {
            dests.push_back(to);
            std::sort(dests.begin(), dests.end());
        }
    }

    /**
     * @brief Destination states reachable from `from` on input `symbol`.
     */
    [[nodiscard]] std::span<const std::size_t> transitions(std::size_t from, std::size_t symbol) const {
        if (from >= state_count_ || symbol >= alphabet_size_) {
            throw std::out_of_range("NFA transition lookup out of bounds.");
        }
        return transitions_[from * alphabet_size_ + symbol];
    }

    /**
     * @brief Destination states reachable from `from` via \(\varepsilon\)-transitions.
     */
    [[nodiscard]] std::span<const std::size_t> epsilon_transitions(std::size_t from) const {
        if (from >= state_count_) {
            throw std::out_of_range("NFA epsilon lookup out of bounds.");
        }
        return epsilon_transitions_[from];
    }

private:
    std::size_t state_count_{0};
    std::size_t alphabet_size_{0};
    std::size_t start_state_{0};
    std::vector<std::vector<std::size_t>> transitions_;
    std::vector<std::vector<std::size_t>> epsilon_transitions_;
    std::vector<bool> accepting_;
};

} // namespace discretex::automata
