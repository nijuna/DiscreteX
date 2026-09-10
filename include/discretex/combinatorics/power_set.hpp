#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <span>
#include <ranges>
#include <bit>
#include <stdexcept>

namespace discretex::views {

enum class subset_ordering {
    binary,
    gray_code
};

class power_set_view : public std::ranges::view_interface<power_set_view> {
public:
    struct sentinel {};

    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::span<const std::size_t>;
        using difference_type = std::ptrdiff_t;

        iterator() = default;
        iterator(std::size_t n, subset_ordering ord) 
            : n_(n), ordering_(ord), current_counter_(0), max_counter_(1ULL << n) {
            decode_current();
        }

        [[nodiscard]] std::span<const std::size_t> operator*() const noexcept {
            return current_subset_;
        }

        iterator& operator++() {
            ++current_counter_;
            if (current_counter_ < max_counter_) {
                decode_current();
            }
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        [[nodiscard]] bool operator==(sentinel) const noexcept {
            return current_counter_ >= max_counter_;
        }

    private:
        void decode_current() {
            current_subset_.clear();
            uint64_t mask = current_counter_;
            if (ordering_ == subset_ordering::gray_code) {
                mask = current_counter_ ^ (current_counter_ >> 1ULL);
            }

            while (mask != 0ULL) {
                int bit = std::countr_zero(mask);
                current_subset_.push_back(static_cast<std::size_t>(bit));
                mask &= (mask - 1ULL);
            }
        }

        std::size_t n_{0};
        subset_ordering ordering_{subset_ordering::binary};
        uint64_t current_counter_{0};
        uint64_t max_counter_{0};
        std::vector<std::size_t> current_subset_;
    };

    explicit power_set_view(std::size_t n, subset_ordering ord = subset_ordering::binary) 
        : n_(n), ordering_(ord) {
        if (n > 63) {
            throw std::invalid_argument("power_set_view requires n <= 63 to avoid 64-bit integer overflow");
        }
    }

    [[nodiscard]] iterator begin() const { return iterator(n_, ordering_); }
    [[nodiscard]] sentinel end() const noexcept { return sentinel{}; }
    [[nodiscard]] std::size_t size() const noexcept { return 1ULL << n_; }

private:
    std::size_t n_{0};
    subset_ordering ordering_{subset_ordering::binary};
};

inline auto power_set(std::size_t n, subset_ordering ord = subset_ordering::binary) {
    return power_set_view(n, ord);
}

} // namespace discretex::views
