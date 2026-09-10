#pragma once
#include <ranges>
#include "../core/domain.hpp"

namespace discretex::views {

// Projects a range of index subsets into a lazy range of domain-element views
template <FiniteDomain Dom, std::ranges::input_range SubsetRange>
auto project_subsets(const Dom& domain, SubsetRange&& subsets) {
    return std::forward<SubsetRange>(subsets) | std::views::transform([&domain](auto index_span) {
        return index_span | std::views::transform([&domain](std::size_t idx) -> decltype(auto) {
            return domain.from_index(idx);
        });
    });
}

} // namespace discretex::views
