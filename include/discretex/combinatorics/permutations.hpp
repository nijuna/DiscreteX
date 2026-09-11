#pragma once

#include <cstddef>
#include <vector>
#include <span>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <ranges>
#include "counting.hpp"
#include "../core/domain.hpp"

namespace discretex::combinatorics {

// Range-based view generating all permutations of {0, ..., n-1} in lexicographical order.
// Each iteration yields a zero-allocation std::span<const std::size_t>.
class permutations_view {
public:
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::span<const std::size_t>;
        using difference_type = std::ptrdiff_t;
        using pointer = const std::span<const std::size_t>*;
        using reference = std::span<const std::size_t>;

        iterator() : done_(true) {}

        explicit iterator(std::size_t n) : n_(n), done_(false) {
            current_.resize(n);
            std::iota(current_.begin(), current_.end(), 0);
        }

        reference operator*() const noexcept {
            return std::span<const std::size_t>(current_.data(), current_.size());
        }

        iterator& operator++() {
            if (!std::next_permutation(current_.begin(), current_.end())) {
                done_ = true;
            }
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const noexcept {
            if (done_ && other.done_) return true;
            if (done_ != other.done_) return false;
            return current_ == other.current_;
        }

        bool operator!=(const iterator& other) const noexcept {
            return !(*this == other);
        }

    private:
        std::size_t n_{0};
        bool done_{false};
        std::vector<std::size_t> current_;
    };

    permutations_view() : n_(0) {}
    explicit permutations_view(std::size_t n) : n_(n) {}

    iterator begin() const { return iterator(n_); }
    iterator end() const { return iterator(); }

    std::size_t size() const {
        return factorial(n_);
    }

    std::size_t size_elements() const noexcept {
        return n_;
    }

private:
    std::size_t n_{0};
};

} // namespace discretex::combinatorics

namespace discretex::views {

// Lazy projection of index permutations onto arbitrary mapped domains
template <FiniteDomain Dom>
inline auto project_permutations(const Dom& domain, const combinatorics::permutations_view& view) {
    return view | std::views::transform([&domain](std::span<const std::size_t> indices) {
        using ValueType = typename Dom::value_type;
        std::vector<ValueType> mapped;
        mapped.reserve(indices.size());
        for (std::size_t idx : indices) {
            mapped.push_back(domain.from_index(idx));
        }
        return mapped;
    });
}

} // namespace discretex::views
