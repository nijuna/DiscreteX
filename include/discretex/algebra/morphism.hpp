#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>
#include "operation_table.hpp"
#include "monoid.hpp"
#include "group.hpp"

namespace discretex::algebra {

// Checks whether a finite map: [0, |A|) -> [0, |B|) is an algebraic homomorphism
// between operation tables A and B: map[A(x, y)] == B(map[x], map[y]).
template <FiniteDomain Dom1, FiniteDomain Dom2>
bool is_homomorphism(const std::vector<std::size_t>& map,
                     const operation_table<Dom1>& A,
                     const operation_table<Dom2>& B) noexcept {
    std::size_t n_a = A.size();
    std::size_t n_b = B.size();
    if (map.size() != n_a) return false;

    for (std::size_t x = 0; x < n_a; ++x) {
        if (map[x] >= n_b) return false;
        for (std::size_t y = 0; y < n_a; ++y) {
            std::size_t lhs = map[A(x, y)];
            std::size_t rhs = B(map[x], map[y]);
            if (lhs != rhs) {
                return false;
            }
        }
    }
    return true;
}

// Checks whether a map is a monoid homomorphism (preserves operation and identity).
template <FiniteDomain Dom1, FiniteDomain Dom2>
bool is_homomorphism(const std::vector<std::size_t>& map,
                     const finite_monoid<Dom1>& A,
                     const finite_monoid<Dom2>& B) noexcept {
    if (map.size() != A.size()) return false;
    if (map[A.identity()] != B.identity()) return false;
    return is_homomorphism(map, A.operation(), B.operation());
}

// Checks whether a map is a group homomorphism.
template <FiniteDomain Dom1, FiniteDomain Dom2>
bool is_homomorphism(const std::vector<std::size_t>& map,
                     const finite_group<Dom1>& A,
                     const finite_group<Dom2>& B) noexcept {
    if (map.size() != A.size()) return false;
    return is_homomorphism(map, A.operation(), B.operation());
}

// Checks if a map is injective (one-to-one).
inline bool is_injective(const std::vector<std::size_t>& map) {
    if (map.empty()) return true;
    auto sorted = map;
    std::sort(sorted.begin(), sorted.end());
    return std::adjacent_find(sorted.begin(), sorted.end()) == sorted.end();
}

// Checks if a map is surjective (onto) given a known codomain size.
inline bool is_surjective(const std::vector<std::size_t>& map, std::size_t codomain_size) {
    if (map.size() < codomain_size) return false;
    std::vector<bool> seen(codomain_size, false);
    std::size_t count = 0;
    for (std::size_t img : map) {
        if (img < codomain_size && !seen[img]) {
            seen[img] = true;
            ++count;
        }
    }
    return count == codomain_size;
}

// Checks if a map is bijective (one-to-one and onto).
inline bool is_bijective(const std::vector<std::size_t>& map, std::size_t codomain_size) {
    return map.size() == codomain_size && is_injective(map);
}

// Checks if a map is an isomorphism between operation tables.
template <FiniteDomain Dom1, FiniteDomain Dom2>
bool is_isomorphism(const std::vector<std::size_t>& map,
                    const operation_table<Dom1>& A,
                    const operation_table<Dom2>& B) noexcept {
    return is_bijective(map, B.size()) && is_homomorphism(map, A, B);
}

// Checks if a map is an isomorphism between finite monoids.
template <FiniteDomain Dom1, FiniteDomain Dom2>
bool is_isomorphism(const std::vector<std::size_t>& map,
                    const finite_monoid<Dom1>& A,
                    const finite_monoid<Dom2>& B) noexcept {
    return is_bijective(map, B.size()) && is_homomorphism(map, A, B);
}

// Checks if a map is an isomorphism between finite groups.
template <FiniteDomain Dom1, FiniteDomain Dom2>
bool is_isomorphism(const std::vector<std::size_t>& map,
                    const finite_group<Dom1>& A,
                    const finite_group<Dom2>& B) noexcept {
    return is_bijective(map, B.size()) && is_homomorphism(map, A, B);
}

// Extracts the kernel of a group homomorphism: {x in G | map[x] == e_H}.
inline std::vector<std::size_t> kernel(const std::vector<std::size_t>& map,
                                       std::size_t codomain_identity) {
    std::vector<std::size_t> ker;
    for (std::size_t x = 0; x < map.size(); ++x) {
        if (map[x] == codomain_identity) {
            ker.push_back(x);
        }
    }
    return ker;
}

// Extracts the image (range) of a map: {map[x] | x in G}, sorted and deduplicated.
inline std::vector<std::size_t> image(const std::vector<std::size_t>& map) {
    auto img = map;
    std::sort(img.begin(), img.end());
    img.erase(std::unique(img.begin(), img.end()), img.end());
    return img;
}

} // namespace discretex::algebra
