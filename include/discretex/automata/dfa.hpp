#pragma once

#include <cstddef>
#include <vector>
#include <span>
#include <stdexcept>
#include <concepts>
#include <ranges>
#include <initializer_list>

namespace discretex::automata {

/**
 * @brief Deterministic Finite Automaton (DFA) over a finite indexed alphabet.
 *
 * Implements a total, deterministic state transition machine where:
 * - States are indexed in [0, state_count).
 * - Alphabet symbols are indexed in [0, alphabet_size).
 * - Transitions are stored in a dense table of size (state_count * alphabet_size).
 * - Totality is maintained: every (state, symbol) pair has a deterministic transition.
 */
class dfa {
public:
    dfa() = default;

    /**
     * @brief Construct a DFA with specified state count and alphabet size.
     * @param state_count Number of states |Q|.
     * @param alphabet_size Cardinality of alphabet |\Sigma|.
     * @param start_state Initial state q_0 in [0, state_count).
     */
    dfa(std::size_t state_count, std::size_t alphabet_size, std::size_t start_state = 0)
        : state_count_(state_count),
          alphabet_size_(alphabet_size),
          start_state_(start_state),
          transitions_(state_count * alphabet_size, 0),
          accepting_(state_count, false) {
        if (state_count > 0 && start_state >= state_count) {
            throw std::invalid_argument("DFA start state index out of bounds.");
        }
    }

    [[nodiscard]] std::size_t state_count() const noexcept { return state_count_; }
    [[nodiscard]] std::size_t alphabet_size() const noexcept { return alphabet_size_; }
    [[nodiscard]] std::size_t start_state() const noexcept { return start_state_; }

    void set_start_state(std::size_t q) {
        if (q >= state_count_) {
            throw std::invalid_argument("DFA start state index out of bounds.");
        }
        start_state_ = q;
    }

    [[nodiscard]] bool is_accepting(std::size_t q) const {
        if (q >= state_count_) {
            throw std::out_of_range("DFA state index out of bounds.");
        }
        return accepting_[q];
    }

    void set_accepting(std::size_t q, bool val = true) {
        if (q >= state_count_) {
            throw std::out_of_range("DFA state index out of bounds.");
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
     * @brief Set deterministic transition delta(from, symbol) = to.
     */
    void set_transition(std::size_t from, std::size_t symbol, std::size_t to) {
        if (from >= state_count_ || to >= state_count_) {
            throw std::out_of_range("DFA transition state index out of bounds.");
        }
        if (symbol >= alphabet_size_) {
            throw std::out_of_range("DFA transition symbol index out of bounds.");
        }
        transitions_[from * alphabet_size_ + symbol] = to;
    }

    /**
     * @brief Evaluate single transition delta(from, symbol).
     */
    [[nodiscard]] std::size_t transition(std::size_t from, std::size_t symbol) const {
        if (from >= state_count_ || symbol >= alphabet_size_) {
            throw std::out_of_range("DFA transition lookup out of bounds.");
        }
        return transitions_[from * alphabet_size_ + symbol];
    }

    /**
     * @brief Synonym for transition(from, symbol).
     */
    [[nodiscard]] std::size_t step(std::size_t from, std::size_t symbol) const {
        return transition(from, symbol);
    }

    /**
     * @brief Test whether a word of symbol indices is accepted by this DFA.
     */
    template <typename Range>
        requires std::ranges::input_range<Range> &&
                 std::convertible_to<std::ranges::range_value_t<Range>, std::size_t>
    [[nodiscard]] bool accepts(const Range& word) const {
        if (state_count_ == 0) return false;
        std::size_t current = start_state_;
        for (auto sym : word) {
            std::size_t a = static_cast<std::size_t>(sym);
            if (a >= alphabet_size_) {
                return false;
            }
            current = transitions_[current * alphabet_size_ + a];
        }
        return accepting_[current];
    }

    [[nodiscard]] bool accepts(std::initializer_list<std::size_t> word) const {
        return accepts<std::initializer_list<std::size_t>>(word);
    }

    [[nodiscard]] bool operator==(const dfa& other) const noexcept = default;

private:
    std::size_t state_count_{0};
    std::size_t alphabet_size_{0};
    std::size_t start_state_{0};
    std::vector<std::size_t> transitions_;
    std::vector<bool> accepting_;
};

} // namespace discretex::automata
