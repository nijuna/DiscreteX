#pragma once

#include <concepts>
#include <numeric>
#include <cmath>
#include <cstdlib>

namespace discretex::number_theory {

template <std::integral T>
struct extended_gcd_result {
    T gcd;
    T x;
    T y;

    bool operator==(const extended_gcd_result& o) const = default;
};

template <std::signed_integral T>
inline extended_gcd_result<T> extended_gcd(T a, T b) {
    T x0 = 1, y0 = 0;
    T x1 = 0, y1 = 1;
    T s_a = (a < 0) ? -1 : 1;
    T s_b = (b < 0) ? -1 : 1;
    a = (a < 0) ? -a : a;
    b = (b < 0) ? -b : b;

    while (b != 0) {
        T q = a / b;
        T r = a % b;
        a = b;
        b = r;

        T next_x = x0 - q * x1;
        x0 = x1;
        x1 = next_x;

        T next_y = y0 - q * y1;
        y0 = y1;
        y1 = next_y;
    }
    return {a, x0 * s_a, y0 * s_b};
}

template <std::integral T>
inline T gcd(T a, T b) {
    return std::gcd(a, b);
}

template <std::integral T>
inline T lcm(T a, T b) {
    return std::lcm(a, b);
}

template <std::signed_integral T>
struct diophantine_result {
    bool has_solution{false};
    T gcd{0};
    T x0{0};
    T y0{0};
    T dx{0}; // step for x: b / gcd
    T dy{0}; // step for y: -a / gcd
};

// Solves a * x + b * y = c
template <std::signed_integral T>
inline diophantine_result<T> solve_diophantine(T a, T b, T c) {
    if (a == 0 && b == 0) {
        if (c == 0) {
            return {true, 0, 0, 0, 0, 0};
        }
        return {false, 0, 0, 0, 0, 0};
    }

    auto eg = extended_gcd(a, b);
    if (eg.gcd == 0 || (c % eg.gcd) != 0) {
        return {false, eg.gcd, 0, 0, 0, 0};
    }

    T factor = c / eg.gcd;
    T x0 = eg.x * factor;
    T y0 = eg.y * factor;
    T dx = b / eg.gcd;
    T dy = -a / eg.gcd;

    return {true, eg.gcd, x0, y0, dx, dy};
}

} // namespace discretex::number_theory
