#pragma once
#include <concepts>
#include <ranges>
#include <cstddef>
#include "domain.hpp"

namespace discretex::concepts {

// Basic Relation: Belongs to a domain and supports membership test (u, v)
template <typename R>
concept Relation = requires(const R& r, std::size_t u, std::size_t v) {
    typename R::domain_type;
    requires FiniteDomain<typename R::domain_type>;
    { r.domain() } -> std::same_as<const typename R::domain_type&>;
    { r.domain_size() } -> std::convertible_to<std::size_t>;
    { r.contains(u, v) } -> std::convertible_to<bool>;
};

// Forward fiber capability: {v | u R v} yielding indices
template <typename R>
concept ForwardRelation = Relation<R> && requires(const R& r, std::size_t u) {
    { r.out_neighbors(u) } -> std::ranges::forward_range;
    requires std::convertible_to<
        std::ranges::range_value_t<decltype(r.out_neighbors(u))>,
        std::size_t>;
};

// Bidirectional fiber capability: yields both out-neighbors and in-neighbors
template <typename R>
concept BidirectionalRelation = ForwardRelation<R> && requires(const R& r, std::size_t v) {
    { r.in_neighbors(v) } -> std::ranges::forward_range;
    requires std::convertible_to<
        std::ranges::range_value_t<decltype(r.in_neighbors(v))>,
        std::size_t>;
};

// Graph Concepts: An index-level directed graph IS a relation
template <typename G>
concept ForwardGraph = ForwardRelation<G>;

template <typename G>
concept BidirectionalGraph = BidirectionalRelation<G>;

} // namespace discretex::concepts
