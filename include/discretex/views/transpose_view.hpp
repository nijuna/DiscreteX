#pragma once
#include <cstddef>
#include "../core/relation_concepts.hpp"

namespace discretex::views {

template <concepts::Relation R>
class transpose_view {
public:
    using domain_type = typename R::domain_type;

    explicit transpose_view(const R& rel) : rel_(rel) {}

    [[nodiscard]] const domain_type& domain() const noexcept {
        return rel_.domain();
    }

    [[nodiscard]] std::size_t domain_size() const noexcept {
        return rel_.domain_size();
    }

    [[nodiscard]] bool contains(std::size_t u, std::size_t v) const noexcept {
        return rel_.contains(v, u);
    }

    auto out_neighbors(std::size_t u) const
        requires concepts::BidirectionalRelation<R> {
        return rel_.in_neighbors(u);
    }

    auto in_neighbors(std::size_t v) const
        requires concepts::BidirectionalRelation<R> {
        return rel_.out_neighbors(v);
    }

private:
    const R& rel_;
};

template <concepts::Relation R>
auto transpose(const R& rel) {
    return transpose_view<R>(rel);
}

} // namespace discretex::views
