#pragma once

#include "dfa.hpp"
#include "nfa.hpp"
#include "algorithms.hpp"

#include <cstddef>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <utility>

namespace discretex::automata {

/**
 * @brief Operation types for abstract regular expression syntax trees.
 */
enum class regex_op {
    empty_lang, ///< \(\emptyset\) (empty language)
    epsilon,    ///< \(\varepsilon\) (empty word)
    literal,    ///< Single alphabet symbol
    concat,     ///< Concatenation \(R_1 \cdot R_2\)
    alt,        ///< Alternation \(R_1 \mid R_2\)
    star        ///< Kleene star \(R_1^*\)
};

/**
 * @brief Node representation of a regular expression AST.
 */
struct regex_node {
    regex_op op{regex_op::empty_lang};
    std::size_t symbol{0};
    std::vector<std::shared_ptr<const regex_node>> children;
};

/**
 * @brief First-class Regular Expression syntax tree representation.
 */
class regex {
public:
    regex()
        : node_(std::make_shared<regex_node>(regex_node{regex_op::empty_lang, 0, {}})) {}

    explicit regex(std::shared_ptr<const regex_node> node)
        : node_(std::move(node)) {}

    [[nodiscard]] regex_op op() const noexcept { return node_->op; }
    [[nodiscard]] std::size_t symbol() const noexcept { return node_->symbol; }
    [[nodiscard]] std::size_t num_children() const noexcept { return node_->children.size(); }

    [[nodiscard]] regex child(std::size_t idx) const {
        return regex(node_->children.at(idx));
    }

    [[nodiscard]] const std::vector<std::shared_ptr<const regex_node>>& raw_children() const noexcept {
        return node_->children;
    }

    [[nodiscard]] std::shared_ptr<const regex_node> node_ptr() const noexcept {
        return node_;
    }

    [[nodiscard]] bool is_empty() const noexcept { return node_->op == regex_op::empty_lang; }
    [[nodiscard]] bool is_epsilon() const noexcept { return node_->op == regex_op::epsilon; }
    [[nodiscard]] bool is_literal() const noexcept { return node_->op == regex_op::literal; }
    [[nodiscard]] bool is_concat() const noexcept { return node_->op == regex_op::concat; }
    [[nodiscard]] bool is_alt() const noexcept { return node_->op == regex_op::alt; }
    [[nodiscard]] bool is_star() const noexcept { return node_->op == regex_op::star; }

    // Factories
    static regex empty() {
        auto n = std::make_shared<regex_node>();
        n->op = regex_op::empty_lang;
        return regex(n);
    }

    static regex eps() {
        auto n = std::make_shared<regex_node>();
        n->op = regex_op::epsilon;
        return regex(n);
    }

    static regex sym(std::size_t s) {
        auto n = std::make_shared<regex_node>();
        n->op = regex_op::literal;
        n->symbol = s;
        return regex(n);
    }

    static regex concat(const regex& a, const regex& b) {
        auto n = std::make_shared<regex_node>();
        n->op = regex_op::concat;
        n->children.push_back(a.node_ptr());
        n->children.push_back(b.node_ptr());
        return regex(n);
    }

    static regex alt(const regex& a, const regex& b) {
        auto n = std::make_shared<regex_node>();
        n->op = regex_op::alt;
        n->children.push_back(a.node_ptr());
        n->children.push_back(b.node_ptr());
        return regex(n);
    }

    static regex star(const regex& a) {
        auto n = std::make_shared<regex_node>();
        n->op = regex_op::star;
        n->children.push_back(a.node_ptr());
        return regex(n);
    }

private:
    std::shared_ptr<const regex_node> node_;
};

// Non-member factories
inline regex empty_lang() { return regex::empty(); }
inline regex epsilon() { return regex::eps(); }
inline regex literal(std::size_t s) { return regex::sym(s); }
inline regex concat(const regex& a, const regex& b) { return regex::concat(a, b); }
inline regex alt(const regex& a, const regex& b) { return regex::alt(a, b); }
inline regex star(const regex& a) { return regex::star(a); }

// Fluent operator overloads
inline regex operator+(const regex& a, const regex& b) { return concat(a, b); }
inline regex operator|(const regex& a, const regex& b) { return alt(a, b); }

// Structural equality
inline bool structurally_equal(const regex& a, const regex& b) {
    if (a.node_ptr() == b.node_ptr()) return true;
    if (a.op() != b.op()) return false;
    if (a.is_literal() && a.symbol() != b.symbol()) return false;
    if (a.num_children() != b.num_children()) return false;
    for (std::size_t i = 0; i < a.num_children(); ++i) {
        if (!structurally_equal(a.child(i), b.child(i))) return false;
    }
    return true;
}

inline bool operator==(const regex& a, const regex& b) {
    return structurally_equal(a, b);
}

// String formatting
inline std::string to_string(const regex& r) {
    switch (r.op()) {
        case regex_op::empty_lang: return "empty";
        case regex_op::epsilon: return "eps";
        case regex_op::literal: return std::to_string(r.symbol());
        case regex_op::concat: return "(" + to_string(r.child(0)) + " . " + to_string(r.child(1)) + ")";
        case regex_op::alt: return "(" + to_string(r.child(0)) + " | " + to_string(r.child(1)) + ")";
        case regex_op::star: return "(" + to_string(r.child(0)) + ")*";
    }
    return "";
}

namespace detail {

struct nfa_fragment {
    std::size_t start{0};
    std::size_t accept{0};
};

class thompson_builder {
public:
    explicit thompson_builder(std::size_t alphabet_size)
        : alphabet_size_(alphabet_size) {}

    std::size_t alloc_state() {
        return num_states_++;
    }

    void add_symbol_edge(std::size_t u, std::size_t symbol, std::size_t v) {
        symbol_edges_.push_back({u, symbol, v});
    }

    void add_epsilon_edge(std::size_t u, std::size_t v) {
        epsilon_edges_.push_back({u, v});
    }

    nfa_fragment build(const regex& r) {
        switch (r.op()) {
            case regex_op::empty_lang: {
                std::size_t s = alloc_state();
                std::size_t t = alloc_state();
                return {s, t};
            }
            case regex_op::epsilon: {
                std::size_t s = alloc_state();
                std::size_t t = alloc_state();
                add_epsilon_edge(s, t);
                return {s, t};
            }
            case regex_op::literal: {
                std::size_t s = alloc_state();
                std::size_t t = alloc_state();
                std::size_t sym = r.symbol();
                if (sym >= alphabet_size_) {
                    throw std::invalid_argument("Literal symbol index exceeds alphabet size.");
                }
                add_symbol_edge(s, sym, t);
                return {s, t};
            }
            case regex_op::concat: {
                auto f1 = build(r.child(0));
                auto f2 = build(r.child(1));
                add_epsilon_edge(f1.accept, f2.start);
                return {f1.start, f2.accept};
            }
            case regex_op::alt: {
                auto f1 = build(r.child(0));
                auto f2 = build(r.child(1));
                std::size_t s = alloc_state();
                std::size_t t = alloc_state();
                add_epsilon_edge(s, f1.start);
                add_epsilon_edge(s, f2.start);
                add_epsilon_edge(f1.accept, t);
                add_epsilon_edge(f2.accept, t);
                return {s, t};
            }
            case regex_op::star: {
                auto f = build(r.child(0));
                std::size_t s = alloc_state();
                std::size_t t = alloc_state();
                add_epsilon_edge(s, f.start);
                add_epsilon_edge(s, t);
                add_epsilon_edge(f.accept, f.start);
                add_epsilon_edge(f.accept, t);
                return {s, t};
            }
        }
        throw std::logic_error("Unknown regex operation type.");
    }

    nfa finalize(const nfa_fragment& root_frag) {
        nfa result(num_states_, alphabet_size_, root_frag.start);
        for (const auto& [u, sym, v] : symbol_edges_) {
            result.add_transition(u, sym, v);
        }
        for (const auto& [u, v] : epsilon_edges_) {
            result.add_epsilon_transition(u, v);
        }
        result.set_accepting(root_frag.accept, true);
        return result;
    }

private:
    std::size_t alphabet_size_{0};
    std::size_t num_states_{0};
    struct symbol_edge { std::size_t u; std::size_t sym; std::size_t v; };
    struct epsilon_edge { std::size_t u; std::size_t v; };
    std::vector<symbol_edge> symbol_edges_;
    std::vector<epsilon_edge> epsilon_edges_;
};

} // namespace detail

/**
 * @brief Thompson inductive construction: compiles a regular expression AST to an equivalent \(\varepsilon\)-NFA.
 */
inline nfa thompson_construction(const regex& r, std::size_t alphabet_size) {
    detail::thompson_builder builder(alphabet_size);
    auto root_frag = builder.build(r);
    return builder.finalize(root_frag);
}

/**
 * @brief Compile a regular expression to an NFA.
 */
inline nfa to_nfa(const regex& r, std::size_t alphabet_size) {
    return thompson_construction(r, alphabet_size);
}

/**
 * @brief Compile a regular expression to an equivalent total DFA via Thompson construction and subset construction.
 */
inline dfa to_dfa(const regex& r, std::size_t alphabet_size) {
    return determinize(to_nfa(r, alphabet_size));
}

/**
 * @brief End-to-end compilation: \(Regex \xrightarrow{\text{Thompson}} NFA \xrightarrow{\text{Powerset}} DFA \xrightarrow{\text{Hopcroft}} DFA_{\min}\).
 */
inline dfa to_min_dfa(const regex& r, std::size_t alphabet_size) {
    return minimize_dfa(to_dfa(r, alphabet_size));
}

} // namespace discretex::automata
