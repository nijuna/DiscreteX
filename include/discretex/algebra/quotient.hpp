#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include "../core/domain.hpp"
#include "operation_table.hpp"
#include "group.hpp"
#include "morphism.hpp"

namespace discretex::algebra {

// Checks whether H is a normal subgroup of G:
// 1. H is a subgroup of G.
// 2. For all g in G and all h in H, g * h * g^-1 in H.
template <FiniteDomain Dom>
bool is_normal_subgroup(const finite_group<Dom>& G, const std::vector<std::size_t>& H) {
    if (!G.is_subgroup(H)) return false;

    std::size_t n = G.size();
    std::vector<bool> in_h(n, false);
    for (std::size_t h : H) {
        in_h[h] = true;
    }

    for (std::size_t g = 0; g < n; ++g) {
        std::size_t g_inv = G.inverse(g);
        for (std::size_t h : H) {
            std::size_t conj = G.op(G.op(g, h), g_inv);
            if (!in_h[conj]) {
                return false;
            }
        }
    }
    return true;
}

// Extracts left cosets gH = {g * h | h in H} as an equivalence partition of G.
// Returns a pair of:
// 1. cosets: vector of coset member vectors
// 2. projection: map from each g in G to its coset index
template <FiniteDomain Dom>
std::pair<std::vector<std::vector<std::size_t>>, std::vector<std::size_t>>
coset_partition(const finite_group<Dom>& G, const std::vector<std::size_t>& H) {
    if (!G.is_subgroup(H)) {
        throw std::invalid_argument("H must be a subgroup of G to form coset partition");
    }

    std::size_t n = G.size();
    std::vector<std::size_t> projection(n, n); // unassigned
    std::vector<std::vector<std::size_t>> cosets;

    for (std::size_t g = 0; g < n; ++g) {
        if (projection[g] == n) {
            std::size_t coset_id = cosets.size();
            std::vector<std::size_t> coset;
            coset.reserve(H.size());

            for (std::size_t h : H) {
                std::size_t gh = G.op(g, h);
                projection[gh] = coset_id;
                coset.push_back(gh);
            }
            std::sort(coset.begin(), coset.end());
            cosets.push_back(std::move(coset));
        }
    }

    return {cosets, projection};
}

// Result structure representing a quotient group G / H.
struct quotient_group_result {
    finite_group<index_domain> quotient;
    std::vector<std::size_t> projection;          // pi: G -> G/H
    std::vector<std::vector<std::size_t>> cosets; // member elements of each coset
    std::vector<std::size_t> representatives;     // canonical representative of each coset
};

// Constructs the quotient group G / H given a normal subgroup H <= G.
// Validates normality, partitions into cosets, and populates the quotient operation table.
template <FiniteDomain Dom>
quotient_group_result quotient_group(const finite_group<Dom>& G, const std::vector<std::size_t>& H) {
    if (!is_normal_subgroup(G, H)) {
        throw std::invalid_argument("H must be a normal subgroup of G to form a quotient group");
    }

    auto [cosets, projection] = coset_partition(G, H);
    std::size_t m = cosets.size();

    std::vector<std::size_t> reps(m);
    for (std::size_t i = 0; i < m; ++i) {
        reps[i] = cosets[i].front(); // canonical representative is lowest index
    }

    // Build quotient operation table: (aH)(bH) = (ab)H
    auto q_op = operation_table<index_domain>::from_callable(
        index_domain(m),
        [&](std::size_t i, std::size_t j) {
            std::size_t rep_prod = G.op(reps[i], reps[j]);
            return projection[rep_prod];
        });

    finite_group<index_domain> q_group(index_domain(m), std::move(q_op));

    return quotient_group_result{
        std::move(q_group),
        std::move(projection),
        std::move(cosets),
        std::move(reps)
    };
}

// Result of the First Isomorphism Theorem certification: G / ker(f) =~ im(f).
struct first_isomorphism_certificate {
    std::vector<std::size_t> kernel;
    std::vector<std::size_t> image;
    quotient_group_result quotient_res;
    finite_group<index_domain> image_group;
    std::vector<std::size_t> isomorphism_map; // maps coset index in G/ker to index in im(f)
    bool is_valid_isomorphism{false};
};

// Implements and certifies the First Isomorphism Theorem for Groups:
// Given a group homomorphism f: G -> K, certifies that G / ker(f) is isomorphic to im(f).
template <FiniteDomain Dom1, FiniteDomain Dom2>
first_isomorphism_certificate certify_first_isomorphism_theorem(
    const finite_group<Dom1>& G,
    const finite_group<Dom2>& K,
    const std::vector<std::size_t>& f) {
    if (!is_homomorphism(f, G, K)) {
        throw std::invalid_argument("f must be a group homomorphism");
    }

    // 1. Extract kernel and image
    auto ker = kernel(f, K.identity());
    auto im = image(f);
    std::size_t m = im.size();

    // 2. Build quotient G / ker(f)
    auto q_res = quotient_group(G, ker);

    // 3. Form image subgroup as an explicit group over index_domain(m)
    std::vector<std::size_t> im_index(K.size(), K.size());
    for (std::size_t i = 0; i < m; ++i) {
        im_index[im[i]] = i;
    }

    auto im_op = operation_table<index_domain>::from_callable(
        index_domain(m),
        [&](std::size_t i, std::size_t j) {
            std::size_t prod = K.op(im[i], im[j]);
            return im_index[prod];
        });

    finite_group<index_domain> im_grp(index_domain(m), std::move(im_op));

    // 4. Construct the induced map f_bar: G/ker -> im(f)
    // f_bar(c) = im_index[f(rep_c)]
    std::vector<std::size_t> f_bar(q_res.quotient.size());
    for (std::size_t c = 0; c < q_res.quotient.size(); ++c) {
        std::size_t rep = q_res.representatives[c];
        f_bar[c] = im_index[f[rep]];
    }

    bool is_iso = is_isomorphism(f_bar, q_res.quotient, im_grp);

    return first_isomorphism_certificate{
        std::move(ker),
        std::move(im),
        std::move(q_res),
        std::move(im_grp),
        std::move(f_bar),
        is_iso
    };
}

} // namespace discretex::algebra
