#pragma once

#include <cstddef>
#include <vector>
#include <span>
#include <iterator>
#include "counting.hpp"

namespace discretex::combinatorics {

// Range-based view generating all integer partitions of n as non-increasing positive integers.
// E.g. for n = 4: [4], [3, 1], [2, 2], [2, 1, 1], [1, 1, 1, 1].
// Yields std::span<const std::size_t> in amortized O(1) per step.
class integer_partitions_view {
public:
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::span<const std::size_t>;
        using difference_type = std::ptrdiff_t;
        using pointer = const std::span<const std::size_t>*;
        using reference = std::span<const std::size_t>;

        iterator() : done_(true) {}

        explicit iterator(std::size_t n) : n_(n), done_(n == 0) {
            if (!done_) {
                current_ = {n};
            }
        }

        reference operator*() const noexcept {
            return std::span<const std::size_t>(current_.data(), current_.size());
        }

        iterator& operator++() {
            advance();
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            advance();
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
        void advance() {
            if (done_ || current_.empty()) {
                done_ = true;
                return;
            }

            // Find rightmost element > 1
            std::size_t i = current_.size();
            while (i > 0 && current_[i - 1] == 1) {
                --i;
            }

            if (i == 0) {
                done_ = true;
                return;
            }

            std::size_t idx = i - 1;
            --current_[idx];

            std::size_t prefix_sum = 0;
            for (std::size_t k = 0; k <= idx; ++k) {
                prefix_sum += current_[k];
            }
            std::size_t rem = n_ - prefix_sum;
            current_.resize(idx + 1);

            std::size_t max_allowed = current_[idx];
            while (rem > max_allowed) {
                current_.push_back(max_allowed);
                rem -= max_allowed;
            }
            if (rem > 0) {
                current_.push_back(rem);
            }
        }

        std::size_t n_{0};
        bool done_{false};
        std::vector<std::size_t> current_;
    };

    explicit integer_partitions_view(std::size_t n) : n_(n) {}

    iterator begin() const { return iterator(n_); }
    iterator end() const { return iterator(); }

    std::size_t target_sum() const noexcept { return n_; }

    std::size_t size() const {
        return partition_number(n_);
    }

private:
    std::size_t n_;
};

} // namespace discretex::combinatorics
