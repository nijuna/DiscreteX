#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include "formula.hpp"

namespace discretex::logic {

class truth_table {
public:
    truth_table(const formula& f, std::size_t num_vars)
        : num_vars_(num_vars), table_(1ULL << num_vars) {
        if (num_vars > 26) {
            throw std::invalid_argument("Truth table exceeds practical variable count limit (max 26)");
        }
        std::size_t total = 1ULL << num_vars;
        for (std::size_t m = 0; m < total; ++m) {
            table_[m] = evaluate(f, static_cast<uint64_t>(m));
        }
    }

    explicit truth_table(const formula& f)
        : truth_table(f, variable_count_span(f)) {}

    std::size_t num_variables() const noexcept { return num_vars_; }
    std::size_t size() const noexcept { return table_.size(); }

    bool operator[](std::size_t assignment) const {
        return table_[assignment];
    }

    bool at(std::size_t assignment) const {
        return table_.at(assignment);
    }

    bool is_tautology() const {
        return std::all_of(table_.begin(), table_.end(), [](bool b) { return b; });
    }

    bool is_contradiction() const {
        return std::none_of(table_.begin(), table_.end(), [](bool b) { return b; });
    }

    bool is_satisfiable() const {
        return std::any_of(table_.begin(), table_.end(), [](bool b) { return b; });
    }

    std::vector<std::size_t> satisfying_assignments() const {
        std::vector<std::size_t> sat;
        for (std::size_t m = 0; m < table_.size(); ++m) {
            if (table_[m]) sat.push_back(m);
        }
        return sat;
    }

    std::vector<std::size_t> falsifying_assignments() const {
        std::vector<std::size_t> unsat;
        for (std::size_t m = 0; m < table_.size(); ++m) {
            if (!table_[m]) unsat.push_back(m);
        }
        return unsat;
    }

    std::size_t count_satisfying() const {
        return std::count(table_.begin(), table_.end(), true);
    }

private:
    std::size_t num_vars_{0};
    std::vector<bool> table_;
};

inline bool is_tautology(const formula& f, std::size_t num_vars) {
    return truth_table(f, num_vars).is_tautology();
}
inline bool is_tautology(const formula& f) {
    return truth_table(f).is_tautology();
}

inline bool is_contradiction(const formula& f, std::size_t num_vars) {
    return truth_table(f, num_vars).is_contradiction();
}
inline bool is_contradiction(const formula& f) {
    return truth_table(f).is_contradiction();
}

inline bool is_satisfiable(const formula& f, std::size_t num_vars) {
    return truth_table(f, num_vars).is_satisfiable();
}
inline bool is_satisfiable(const formula& f) {
    return truth_table(f).is_satisfiable();
}

inline bool equivalent(const formula& a, const formula& b) {
    std::size_t n = std::max(variable_count_span(a), variable_count_span(b));
    truth_table ta(a, n);
    truth_table tb(b, n);
    for (std::size_t i = 0; i < ta.size(); ++i) {
        if (ta[i] != tb[i]) return false;
    }
    return true;
}

inline bool entails(const formula& premise, const formula& conclusion) {
    std::size_t n = std::max(variable_count_span(premise), variable_count_span(conclusion));
    truth_table tp(premise, n);
    truth_table tc(conclusion, n);
    for (std::size_t i = 0; i < tp.size(); ++i) {
        if (tp[i] && !tc[i]) return false;
    }
    return true;
}

} // namespace discretex::logic
