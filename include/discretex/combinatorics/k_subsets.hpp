#pragma once
#include <cstddef>
#include <vector>
#include <span>
#include <ranges>
#include <iterator>

namespace discretex::views {

class k_subsets_view : public std::ranges::view_interface<k_subsets_view> {
public:
    struct sentinel {};

    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::span<const std::size_t>;
        using difference_type = std::ptrdiff_t;

        iterator() = default;
        iterator(std::size_t n, std::size_t k)
            : n_(n), k_(k), current_(k) {
            if (k_ > n_) {
                is_end_ = true;
                return;
            }
            for (std::size_t i = 0; i < k_; ++i) {
                current_[i] = i;
            }
        }

        [[nodiscard]] std::span<const std::size_t> operator*() const noexcept {
            return current_;
        }

        iterator& operator++() {
            advance();
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        [[nodiscard]] bool operator==(sentinel) const noexcept {
            return is_end_;
        }

        [[nodiscard]] bool operator==(const iterator& other) const noexcept {
            if (is_end_ && other.is_end_) return true;
            if (is_end_ != other.is_end_) return false;
            return current_ == other.current_;
        }

    private:
        void advance() {
            if (k_ == 0) {
                is_end_ = true;
                return;
            }

            std::ptrdiff_t i = static_cast<std::ptrdiff_t>(k_) - 1;
            while (i >= 0 && current_[i] == n_ - k_ + static_cast<std::size_t>(i)) {
                --i;
            }

            if (i < 0) {
                is_end_ = true;
                return;
            }

            ++current_[i];
            for (std::size_t j = static_cast<std::size_t>(i) + 1; j < k_; ++j) {
                current_[j] = current_[j - 1] + 1;
            }
        }

        std::size_t n_{0};
        std::size_t k_{0};
        std::vector<std::size_t> current_;
        bool is_end_{false};
    };

    constexpr k_subsets_view(std::size_t n, std::size_t k) noexcept : n_(n), k_(k) {}

    [[nodiscard]] iterator begin() const { return iterator(n_, k_); }
    [[nodiscard]] sentinel end() const noexcept { return sentinel{}; }

private:
    std::size_t n_;
    std::size_t k_;
};

inline auto k_subsets(std::size_t n, std::size_t k) noexcept {
    return k_subsets_view(n, k);
}

} // namespace discretex::views
