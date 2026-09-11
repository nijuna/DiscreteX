#pragma once

#include <cstddef>
#include <vector>
#include <set>
#include <algorithm>
#include <utility>
#include "formula.hpp"
#include "truth_table.hpp"

namespace discretex::logic {

struct literal {
    std::size_t var{0};
    bool negated{false};

    bool operator==(const literal& o) const = default;
    auto operator<=>(const literal& o) const = default;
};

struct clause {
    std::vector<literal> literals;

    bool operator==(const clause& o) const = default;
};

// Negation Normal Form (NNF)
inline formula to_nnf(const formula& f) {
    switch (f.op()) {
        case op_type::variable:
        case op_type::constant:
            return f;

        case op_type::implies_op:
            // A -> B  ==  ~A | B
            return to_nnf(or_(not_(f.child(0)), f.child(1)));

        case op_type::iff_op:
            // A <-> B == (~A | B) & (~B | A)
            return to_nnf(and_(or_(not_(f.child(0)), f.child(1)),
                               or_(not_(f.child(1)), f.child(0))));

        case op_type::and_op: {
            std::vector<formula> children;
            children.reserve(f.num_children());
            for (std::size_t i = 0; i < f.num_children(); ++i) {
                children.push_back(to_nnf(f.child(i)));
            }
            return and_all(children);
        }

        case op_type::or_op: {
            std::vector<formula> children;
            children.reserve(f.num_children());
            for (std::size_t i = 0; i < f.num_children(); ++i) {
                children.push_back(to_nnf(f.child(i)));
            }
            return or_all(children);
        }

        case op_type::not_op: {
            const auto& inner = f.child(0);
            switch (inner.op()) {
                case op_type::constant:
                    return boolean(!inner.const_val());
                case op_type::variable:
                    return f; // literal ~x_i
                case op_type::not_op:
                    // ~~A == A
                    return to_nnf(inner.child(0));
                case op_type::and_op: {
                    // ~(A & B & ...) == ~A | ~B | ...
                    std::vector<formula> neg_children;
                    neg_children.reserve(inner.num_children());
                    for (std::size_t i = 0; i < inner.num_children(); ++i) {
                        neg_children.push_back(to_nnf(not_(inner.child(i))));
                    }
                    return or_all(neg_children);
                }
                case op_type::or_op: {
                    // ~(A | B | ...) == ~A & ~B & ...
                    std::vector<formula> neg_children;
                    neg_children.reserve(inner.num_children());
                    for (std::size_t i = 0; i < inner.num_children(); ++i) {
                        neg_children.push_back(to_nnf(not_(inner.child(i))));
                    }
                    return and_all(neg_children);
                }
                case op_type::implies_op:
                    // ~(A -> B) == A & ~B
                    return to_nnf(and_(inner.child(0), not_(inner.child(1))));
                case op_type::iff_op:
                    // ~(A <-> B) == (A & ~B) | (~A & B)
                    return to_nnf(or_(and_(inner.child(0), not_(inner.child(1))),
                                      and_(not_(inner.child(0)), inner.child(1))));
            }
        }
    }
    return f;
}

// Canonical DNF (Minterm expansion from truth table)
inline formula to_canonical_dnf(const formula& f, std::size_t num_vars) {
    truth_table tt(f, num_vars);
    auto sat = tt.satisfying_assignments();
    if (sat.empty()) return boolean(false);
    if (num_vars == 0) return boolean(true);

    std::vector<formula> minterms;
    minterms.reserve(sat.size());

    for (std::size_t m : sat) {
        std::vector<formula> term_lits;
        term_lits.reserve(num_vars);
        for (std::size_t v = 0; v < num_vars; ++v) {
            bool val = ((m >> v) & 1ULL) != 0;
            if (val) {
                term_lits.push_back(var(v));
            } else {
                term_lits.push_back(not_(var(v)));
            }
        }
        minterms.push_back(and_all(term_lits));
    }
    return or_all(minterms);
}

inline formula to_canonical_dnf(const formula& f) {
    return to_canonical_dnf(f, variable_count_span(f));
}

// Canonical CNF (Maxterm expansion from truth table)
inline formula to_canonical_cnf(const formula& f, std::size_t num_vars) {
    truth_table tt(f, num_vars);
    auto unsat = tt.falsifying_assignments();
    if (unsat.empty()) return boolean(true);
    if (num_vars == 0) return boolean(false);

    std::vector<formula> maxterms;
    maxterms.reserve(unsat.size());

    for (std::size_t m : unsat) {
        std::vector<formula> clause_lits;
        clause_lits.reserve(num_vars);
        for (std::size_t v = 0; v < num_vars; ++v) {
            bool val = ((m >> v) & 1ULL) != 0;
            if (val) {
                clause_lits.push_back(not_(var(v)));
            } else {
                clause_lits.push_back(var(v));
            }
        }
        maxterms.push_back(or_all(clause_lits));
    }
    return and_all(maxterms);
}

inline formula to_canonical_cnf(const formula& f) {
    return to_canonical_cnf(f, variable_count_span(f));
}

// Algebraic DNF and CNF distribution helpers
namespace detail {

// Helper: normalize and clean a set of literals for a conjunction (term)
// Returns false if term contains x and ~x (contradictory)
inline bool clean_term(std::vector<literal>& lits) {
    std::sort(lits.begin(), lits.end());
    lits.erase(std::unique(lits.begin(), lits.end()), lits.end());
    for (std::size_t i = 1; i < lits.size(); ++i) {
        if (lits[i].var == lits[i - 1].var && lits[i].negated != lits[i - 1].negated) {
            return false;
        }
    }
    return true;
}

// Helper: normalize and clean a set of literals for a disjunction (clause)
// Returns false if clause contains x and ~x (tautological)
inline bool clean_clause(std::vector<literal>& lits) {
    std::sort(lits.begin(), lits.end());
    lits.erase(std::unique(lits.begin(), lits.end()), lits.end());
    for (std::size_t i = 1; i < lits.size(); ++i) {
        if (lits[i].var == lits[i - 1].var && lits[i].negated != lits[i - 1].negated) {
            return false; // tautology
        }
    }
    return true;
}

inline std::vector<std::vector<literal>> dnf_terms(const formula& f_nnf) {
    switch (f_nnf.op()) {
        case op_type::constant: {
            if (f_nnf.const_val()) {
                return { {} }; // one empty term (true)
            } else {
                return {};     // zero terms (false)
            }
        }
        case op_type::variable: {
            return { { literal{f_nnf.var_id(), false} } };
        }
        case op_type::not_op: {
            // In NNF, not_op is applied only to a variable
            return { { literal{f_nnf.child(0).var_id(), true} } };
        }
        case op_type::or_op: {
            std::vector<std::vector<literal>> result;
            for (std::size_t i = 0; i < f_nnf.num_children(); ++i) {
                auto sub = dnf_terms(f_nnf.child(i));
                for (auto& term : sub) {
                    result.push_back(std::move(term));
                }
            }
            // Check if any term is empty (true) -> whole disjunction is true
            for (const auto& t : result) {
                if (t.empty()) return { {} };
            }
            return result;
        }
        case op_type::and_op: {
            std::vector<std::vector<literal>> current = { {} };
            for (std::size_t i = 0; i < f_nnf.num_children(); ++i) {
                auto sub = dnf_terms(f_nnf.child(i));
                if (sub.empty()) return {}; // false
                std::vector<std::vector<literal>> next;
                for (const auto& t1 : current) {
                    for (const auto& t2 : sub) {
                        std::vector<literal> merged = t1;
                        merged.insert(merged.end(), t2.begin(), t2.end());
                        if (clean_term(merged)) {
                            next.push_back(std::move(merged));
                        }
                    }
                }
                current = std::move(next);
                if (current.empty()) return {};
            }
            return current;
        }
        default:
            return {};
    }
}

inline std::vector<std::vector<literal>> cnf_clauses(const formula& f_nnf) {
    switch (f_nnf.op()) {
        case op_type::constant: {
            if (f_nnf.const_val()) {
                return {};     // zero clauses (true)
            } else {
                return { {} }; // one empty clause (false)
            }
        }
        case op_type::variable: {
            return { { literal{f_nnf.var_id(), false} } };
        }
        case op_type::not_op: {
            return { { literal{f_nnf.child(0).var_id(), true} } };
        }
        case op_type::and_op: {
            std::vector<std::vector<literal>> result;
            for (std::size_t i = 0; i < f_nnf.num_children(); ++i) {
                auto sub = cnf_clauses(f_nnf.child(i));
                for (auto& clause_item : sub) {
                    result.push_back(std::move(clause_item));
                }
            }
            // Check if any clause is empty (false) -> whole conjunction is false
            for (const auto& c : result) {
                if (c.empty()) return { {} };
            }
            return result;
        }
        case op_type::or_op: {
            std::vector<std::vector<literal>> current = { {} };
            for (std::size_t i = 0; i < f_nnf.num_children(); ++i) {
                auto sub = cnf_clauses(f_nnf.child(i));
                if (sub.empty()) return {}; // true
                std::vector<std::vector<literal>> next;
                for (const auto& c1 : current) {
                    for (const auto& c2 : sub) {
                        std::vector<literal> merged = c1;
                        merged.insert(merged.end(), c2.begin(), c2.end());
                        if (clean_clause(merged)) {
                            next.push_back(std::move(merged));
                        }
                    }
                }
                current = std::move(next);
            }
            return current;
        }
        default:
            return {};
    }
}

} // namespace detail

// Algebraic DNF via distribution
inline formula to_dnf(const formula& f) {
    formula nnf = to_nnf(f);
    auto terms = detail::dnf_terms(nnf);
    if (terms.empty()) return boolean(false);
    if (terms.size() == 1 && terms[0].empty()) return boolean(true);

    std::vector<formula> term_formulas;
    term_formulas.reserve(terms.size());
    for (const auto& term : terms) {
        if (term.empty()) return boolean(true);
        std::vector<formula> lits;
        lits.reserve(term.size());
        for (const auto& lit : term) {
            lits.push_back(lit.negated ? not_(var(lit.var)) : var(lit.var));
        }
        term_formulas.push_back(and_all(lits));
    }
    return or_all(term_formulas);
}

// Algebraic CNF via distribution
inline formula to_cnf(const formula& f) {
    formula nnf = to_nnf(f);
    auto clauses = detail::cnf_clauses(nnf);
    if (clauses.empty()) return boolean(true);
    if (clauses.size() == 1 && clauses[0].empty()) return boolean(false);

    std::vector<formula> clause_formulas;
    clause_formulas.reserve(clauses.size());
    for (const auto& cl : clauses) {
        if (cl.empty()) return boolean(false);
        std::vector<formula> lits;
        lits.reserve(cl.size());
        for (const auto& lit : cl) {
            lits.push_back(lit.negated ? not_(var(lit.var)) : var(lit.var));
        }
        clause_formulas.push_back(or_all(lits));
    }
    return and_all(clause_formulas);
}

} // namespace discretex::logic
