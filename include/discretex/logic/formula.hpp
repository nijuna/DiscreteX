#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <set>
#include <ostream>
#include <stdexcept>
#include <concepts>
#include <type_traits>

namespace discretex::logic {

enum class op_type {
    variable,
    constant,
    not_op,
    and_op,
    or_op,
    implies_op,
    iff_op
};

struct formula_node {
    op_type op{op_type::constant};
    std::size_t var_id{0};
    bool const_val{false};
    std::vector<std::shared_ptr<const formula_node>> children;
};

class formula {
public:
    formula()
        : node_(std::make_shared<formula_node>(formula_node{op_type::constant, 0, false, {}})) {}

    explicit formula(std::shared_ptr<const formula_node> node)
        : node_(std::move(node)) {}

    op_type op() const noexcept { return node_->op; }
    std::size_t var_id() const noexcept { return node_->var_id; }
    bool const_val() const noexcept { return node_->const_val; }

    std::size_t num_children() const noexcept { return node_->children.size(); }

    formula child(std::size_t idx) const {
        return formula(node_->children.at(idx));
    }

    const std::vector<std::shared_ptr<const formula_node>>& raw_children() const noexcept {
        return node_->children;
    }

    bool is_variable() const noexcept { return node_->op == op_type::variable; }
    bool is_constant() const noexcept { return node_->op == op_type::constant; }
    bool is_not() const noexcept { return node_->op == op_type::not_op; }
    bool is_and() const noexcept { return node_->op == op_type::and_op; }
    bool is_or() const noexcept { return node_->op == op_type::or_op; }
    bool is_implies() const noexcept { return node_->op == op_type::implies_op; }
    bool is_iff() const noexcept { return node_->op == op_type::iff_op; }

    std::shared_ptr<const formula_node> node_ptr() const noexcept { return node_; }

private:
    std::shared_ptr<const formula_node> node_;
};

// String representation
inline std::string to_string(const formula& f) {
    switch (f.op()) {
        case op_type::variable:
            return "x" + std::to_string(f.var_id());
        case op_type::constant:
            return f.const_val() ? "T" : "F";
        case op_type::not_op:
            return "~" + to_string(f.child(0));
        case op_type::and_op: {
            if (f.num_children() == 0) return "T";
            std::string s = "(";
            for (std::size_t i = 0; i < f.num_children(); ++i) {
                if (i > 0) s += " & ";
                s += to_string(f.child(i));
            }
            s += ")";
            return s;
        }
        case op_type::or_op: {
            if (f.num_children() == 0) return "F";
            std::string s = "(";
            for (std::size_t i = 0; i < f.num_children(); ++i) {
                if (i > 0) s += " | ";
                s += to_string(f.child(i));
            }
            s += ")";
            return s;
        }
        case op_type::implies_op:
            return "(" + to_string(f.child(0)) + " -> " + to_string(f.child(1)) + ")";
        case op_type::iff_op:
            return "(" + to_string(f.child(0)) + " <-> " + to_string(f.child(1)) + ")";
    }
    return "";
}

inline std::ostream& operator<<(std::ostream& os, const formula& f) {
    return os << to_string(f);
}

// Factories
inline formula var(std::size_t id) {
    auto n = std::make_shared<formula_node>();
    n->op = op_type::variable;
    n->var_id = id;
    return formula(n);
}

inline formula boolean(bool val) {
    auto n = std::make_shared<formula_node>();
    n->op = op_type::constant;
    n->const_val = val;
    return formula(n);
}

inline formula not_(const formula& f) {
    auto n = std::make_shared<formula_node>();
    n->op = op_type::not_op;
    n->children.push_back(f.node_ptr());
    return formula(n);
}

inline formula and_(const formula& a, const formula& b) {
    auto n = std::make_shared<formula_node>();
    n->op = op_type::and_op;
    n->children.push_back(a.node_ptr());
    n->children.push_back(b.node_ptr());
    return formula(n);
}

inline formula or_(const formula& a, const formula& b) {
    auto n = std::make_shared<formula_node>();
    n->op = op_type::or_op;
    n->children.push_back(a.node_ptr());
    n->children.push_back(b.node_ptr());
    return formula(n);
}

inline formula implies_(const formula& a, const formula& b) {
    auto n = std::make_shared<formula_node>();
    n->op = op_type::implies_op;
    n->children.push_back(a.node_ptr());
    n->children.push_back(b.node_ptr());
    return formula(n);
}

inline formula iff_(const formula& a, const formula& b) {
    auto n = std::make_shared<formula_node>();
    n->op = op_type::iff_op;
    n->children.push_back(a.node_ptr());
    n->children.push_back(b.node_ptr());
    return formula(n);
}

inline formula and_all(const std::vector<formula>& list) {
    if (list.empty()) return boolean(true);
    if (list.size() == 1) return list[0];
    auto n = std::make_shared<formula_node>();
    n->op = op_type::and_op;
    for (const auto& item : list) {
        n->children.push_back(item.node_ptr());
    }
    return formula(n);
}

inline formula or_all(const std::vector<formula>& list) {
    if (list.empty()) return boolean(false);
    if (list.size() == 1) return list[0];
    auto n = std::make_shared<formula_node>();
    n->op = op_type::or_op;
    for (const auto& item : list) {
        n->children.push_back(item.node_ptr());
    }
    return formula(n);
}

// Operator overloads
inline formula operator!(const formula& f) { return not_(f); }
inline formula operator&&(const formula& a, const formula& b) { return and_(a, b); }
inline formula operator||(const formula& a, const formula& b) { return or_(a, b); }
inline formula operator&(const formula& a, const formula& b) { return and_(a, b); }
inline formula operator|(const formula& a, const formula& b) { return or_(a, b); }

// Syntactic / structural equality
inline bool structurally_equal(const formula& a, const formula& b) {
    if (a.node_ptr() == b.node_ptr()) return true;
    if (a.op() != b.op()) return false;
    if (a.is_variable()) return a.var_id() == b.var_id();
    if (a.is_constant()) return a.const_val() == b.const_val();
    if (a.num_children() != b.num_children()) return false;
    for (std::size_t i = 0; i < a.num_children(); ++i) {
        if (!structurally_equal(a.child(i), b.child(i))) return false;
    }
    return true;
}

inline bool operator==(const formula& a, const formula& b) {
    return structurally_equal(a, b);
}

// Variable collection
inline void collect_variables_helper(const formula& f, std::set<std::size_t>& vars) {
    if (f.is_variable()) {
        vars.insert(f.var_id());
    } else {
        for (std::size_t i = 0; i < f.num_children(); ++i) {
            collect_variables_helper(f.child(i), vars);
        }
    }
}

inline std::set<std::size_t> collect_variables(const formula& f) {
    std::set<std::size_t> vars;
    collect_variables_helper(f, vars);
    return vars;
}

inline std::size_t variable_count_span(const formula& f) {
    auto vars = collect_variables(f);
    if (vars.empty()) return 0;
    return (*vars.rbegin()) + 1;
}

// Evaluation with 64-bit assignment bitmask
// Variable i corresponds to bit ((assignment >> i) & 1).
inline bool evaluate(const formula& f, uint64_t assignment_bitmask) {
    switch (f.op()) {
        case op_type::constant:
            return f.const_val();
        case op_type::variable: {
            if (f.var_id() >= 64) {
                throw std::out_of_range("Variable ID exceeds 63 for 64-bit bitmask assignment");
            }
            return ((assignment_bitmask >> f.var_id()) & 1ULL) != 0ULL;
        }
        case op_type::not_op:
            return !evaluate(f.child(0), assignment_bitmask);
        case op_type::and_op: {
            for (std::size_t i = 0; i < f.num_children(); ++i) {
                if (!evaluate(f.child(i), assignment_bitmask)) return false;
            }
            return true;
        }
        case op_type::or_op: {
            for (std::size_t i = 0; i < f.num_children(); ++i) {
                if (evaluate(f.child(i), assignment_bitmask)) return true;
            }
            return false;
        }
        case op_type::implies_op:
            return !evaluate(f.child(0), assignment_bitmask) || evaluate(f.child(1), assignment_bitmask);
        case op_type::iff_op:
            return evaluate(f.child(0), assignment_bitmask) == evaluate(f.child(1), assignment_bitmask);
    }
    return false;
}

template <typename VarEvalFunc>
requires std::is_invocable_r_v<bool, VarEvalFunc, std::size_t>
inline bool evaluate_custom(const formula& f, VarEvalFunc&& eval_var) {
    switch (f.op()) {
        case op_type::constant:
            return f.const_val();
        case op_type::variable:
            return eval_var(f.var_id());
        case op_type::not_op:
            return !evaluate_custom(f.child(0), eval_var);
        case op_type::and_op: {
            for (std::size_t i = 0; i < f.num_children(); ++i) {
                if (!evaluate_custom(f.child(i), eval_var)) return false;
            }
            return true;
        }
        case op_type::or_op: {
            for (std::size_t i = 0; i < f.num_children(); ++i) {
                if (evaluate_custom(f.child(i), eval_var)) return true;
            }
            return false;
        }
        case op_type::implies_op:
            return !evaluate_custom(f.child(0), eval_var) || evaluate_custom(f.child(1), eval_var);
        case op_type::iff_op:
            return evaluate_custom(f.child(0), eval_var) == evaluate_custom(f.child(1), eval_var);
    }
    return false;
}

} // namespace discretex::logic
