#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <bit>
#include <iterator>

namespace discretex::storage {

class bit_matrix {
public:
    explicit bit_matrix(std::size_t rows = 0, std::size_t cols = 0)
        : rows_(rows), cols_(cols), words_per_row_((cols + 63) / 64) {
        data_.resize(rows_ * words_per_row_, 0ULL);
    }

    [[nodiscard]] std::size_t rows() const noexcept { return rows_; }
    [[nodiscard]] std::size_t cols() const noexcept { return cols_; }
    [[nodiscard]] std::size_t words_per_row() const noexcept { return words_per_row_; }

    [[nodiscard]] bool test(std::size_t r, std::size_t c) const noexcept {
        if (r >= rows_ || c >= cols_) return false;
        return (data_[r * words_per_row_ + (c / 64)] & (1ULL << (c % 64))) != 0ULL;
    }

    void set(std::size_t r, std::size_t c, bool val = true) noexcept {
        if (r >= rows_ || c >= cols_) return;
        std::size_t idx = r * words_per_row_ + (c / 64);
        uint64_t mask = 1ULL << (c % 64);
        if (val) data_[idx] |= mask;
        else     data_[idx] &= ~mask;
    }

    // Word load with tail bit masking applied at boundary
    [[nodiscard]] uint64_t load_word(std::size_t r, std::size_t w) const noexcept {
        if (r >= rows_ || w >= words_per_row_) return 0ULL;
        uint64_t word = data_[r * words_per_row_ + w];
        if (w + 1 == words_per_row_ && (cols_ % 64 != 0)) {
            word &= ((1ULL << (cols_ % 64)) - 1ULL);
        }
        return word;
    }

    void row_or(std::size_t r1, std::size_t r2) noexcept {
        if (r1 >= rows_ || r2 >= rows_) return;
        std::size_t offset1 = r1 * words_per_row_;
        std::size_t offset2 = r2 * words_per_row_;
        for (std::size_t w = 0; w < words_per_row_; ++w) {
            data_[offset1 + w] |= data_[offset2 + w];
        }
    }

private:
    std::size_t rows_{0};
    std::size_t cols_{0};
    std::size_t words_per_row_{0};
    std::vector<uint64_t> data_;
};

// Fiber view with dedicated sentinel and masked word ingestion
class bit_row_fiber_view {
public:
    struct sentinel {};

    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        iterator() = default;
        iterator(const bit_matrix* matrix, std::size_t row)
            : matrix_(matrix), row_(row), words_per_row_(matrix ? matrix->words_per_row() : 0) {
            if (matrix_) {
                load_next_active();
            }
        }

        std::size_t operator*() const noexcept { return current_col_; }

        iterator& operator++() {
            advance();
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(sentinel) const noexcept {
            return current_word_idx_ >= words_per_row_ && active_word_ == 0ULL;
        }

        bool operator==(const iterator& other) const noexcept {
            if ((*this == sentinel{}) && (other == sentinel{})) return true;
            return matrix_ == other.matrix_ && row_ == other.row_ &&
                   current_word_idx_ == other.current_word_idx_ &&
                   active_word_ == other.active_word_;
        }

    private:
        void load_next_active() {
            while (current_word_idx_ < words_per_row_) {
                active_word_ = matrix_->load_word(row_, current_word_idx_);
                if (active_word_ != 0ULL) {
                    extract_bit();
                    return;
                }
                ++current_word_idx_;
            }
        }

        void advance() {
            if (active_word_ != 0ULL) {
                extract_bit();
            } else {
                ++current_word_idx_;
                load_next_active();
            }
        }

        void extract_bit() {
            int tz = std::countr_zero(active_word_);
            current_col_ = current_word_idx_ * 64 + static_cast<std::size_t>(tz);
            active_word_ &= (active_word_ - 1ULL); // Clear extracted bit
        }

        const bit_matrix* matrix_{nullptr};
        std::size_t row_{0};
        std::size_t words_per_row_{0};
        std::size_t current_word_idx_{0};
        uint64_t active_word_{0ULL};
        std::size_t current_col_{0};
    };

    bit_row_fiber_view(const bit_matrix& matrix, std::size_t row)
        : matrix_(&matrix), row_(row) {}

    [[nodiscard]] iterator begin() const noexcept { return iterator(matrix_, row_); }
    [[nodiscard]] sentinel end() const noexcept { return sentinel{}; }

private:
    const bit_matrix* matrix_;
    std::size_t row_;
};

} // namespace discretex::storage
