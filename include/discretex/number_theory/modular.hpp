#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <ostream>
#include "euclidean.hpp"

namespace discretex::number_theory {

#if defined(__SIZEOF_INT128__)
__extension__ typedef unsigned __int128 uint128_t;
__extension__ typedef __int128 int128_t;
#endif

// Safe 64-bit modular addition
inline uint64_t add_mod(uint64_t a, uint64_t b, uint64_t m) {
    if (m <= 1) return 0;
    a %= m;
    b %= m;
    return (a >= m - b) ? (a - (m - b)) : (a + b);
}

// Safe 64-bit modular subtraction
inline uint64_t sub_mod(uint64_t a, uint64_t b, uint64_t m) {
    if (m <= 1) return 0;
    a %= m;
    b %= m;
    return (a >= b) ? (a - b) : (m - (b - a));
}

// Safe 64-bit modular multiplication using 128-bit integer
inline uint64_t mul_mod(uint64_t a, uint64_t b, uint64_t m) {
    if (m <= 1) return 0;
#if defined(__SIZEOF_INT128__)
    return static_cast<uint64_t>((static_cast<uint128_t>(a % m) * (b % m)) % m);
#else
    uint64_t res = 0;
    a %= m;
    b %= m;
    while (b > 0) {
        if (b & 1) res = add_mod(res, a, m);
        a = add_mod(a, a, m);
        b >>= 1;
    }
    return res;
#endif
}

// Modular exponentiation (base^exp mod m)
inline uint64_t power_mod(uint64_t base, uint64_t exp, uint64_t mod) {
    if (mod <= 1) return 0;
    uint64_t result = 1 % mod;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) {
            result = mul_mod(result, base, mod);
        }
        base = mul_mod(base, base, mod);
        exp >>= 1;
    }
    return result;
}

// Modular inverse via Extended Euclidean Algorithm
inline std::optional<uint64_t> mod_inverse(uint64_t a, uint64_t m) {
    if (m <= 1) return std::nullopt;
    auto eg = extended_gcd(static_cast<int64_t>(a % m), static_cast<int64_t>(m));
    if (eg.gcd != 1) return std::nullopt;
    int64_t inv = eg.x % static_cast<int64_t>(m);
    if (inv < 0) inv += static_cast<int64_t>(m);
    return static_cast<uint64_t>(inv);
}

// Dynamic Modulo Integer (Element of Z / mZ)
class dynamic_mod_int {
public:
    dynamic_mod_int(uint64_t value, uint64_t mod)
        : val_(mod > 0 ? value % mod : 0), mod_(mod) {
        if (mod == 0) {
            throw std::invalid_argument("Modulus cannot be zero");
        }
    }

    uint64_t value() const noexcept { return val_; }
    uint64_t modulus() const noexcept { return mod_; }

    dynamic_mod_int operator+(const dynamic_mod_int& o) const {
        check_mod(o);
        return dynamic_mod_int(add_mod(val_, o.val_, mod_), mod_);
    }

    dynamic_mod_int operator-(const dynamic_mod_int& o) const {
        check_mod(o);
        return dynamic_mod_int(sub_mod(val_, o.val_, mod_), mod_);
    }

    dynamic_mod_int operator*(const dynamic_mod_int& o) const {
        check_mod(o);
        return dynamic_mod_int(mul_mod(val_, o.val_, mod_), mod_);
    }

    dynamic_mod_int operator/(const dynamic_mod_int& o) const {
        check_mod(o);
        auto inv = mod_inverse(o.val_, mod_);
        if (!inv.has_value()) {
            throw std::domain_error("Division by non-invertible element in Z/mZ");
        }
        return dynamic_mod_int(mul_mod(val_, *inv, mod_), mod_);
    }

    dynamic_mod_int pow(uint64_t exp) const {
        return dynamic_mod_int(power_mod(val_, exp, mod_), mod_);
    }

    std::optional<dynamic_mod_int> inv() const {
        auto inv_val = mod_inverse(val_, mod_);
        if (!inv_val.has_value()) return std::nullopt;
        return dynamic_mod_int(*inv_val, mod_);
    }

    bool operator==(const dynamic_mod_int& o) const noexcept {
        return val_ == o.val_ && mod_ == o.mod_;
    }

    bool operator!=(const dynamic_mod_int& o) const noexcept {
        return !(*this == o);
    }

private:
    void check_mod(const dynamic_mod_int& o) const {
        if (mod_ != o.mod_) {
            throw std::invalid_argument("Moduli must match for operations in Z/mZ");
        }
    }

    uint64_t val_{0};
    uint64_t mod_{1};
};

inline std::ostream& operator<<(std::ostream& os, const dynamic_mod_int& x) {
    return os << x.value() << " (mod " << x.modulus() << ")";
}

} // namespace discretex::number_theory
