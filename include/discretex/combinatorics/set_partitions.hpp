#pragma once

#include <cstddef>
#include <vector>
#include <span>
#include <algorithm>
#include <stdexcept>
#include <iterator>

namespace discretex::combinatorics {

// Helper: Count number of blocks in an RGS
inline std::size_t block_count(std::span<const std::size_t> rgs) {
    if (rgs.empty()) return 0;
    std::size_t max_val = 0;
    for (std::size_t v : rgs) {
        if (v > max_val) max_val = v;
    }
    return max_val + 1;
}

// Helper: Convert Restricted Growth String (RGS) to explicit blocks
inline std::vector<std::vector<std::size_t>> rgs_to_blocks(std::span<const std::size_t> rgs) {
    if (rgs.empty()) return {};
    std::size_t k = block_count(rgs);
    std::vector<std::vector<std::size_t>> blocks(k);
    for (std::size_t i = 0; i < rgs.size(); ++i) {
        blocks[rgs[i]].push_back(i);
    }
    return blocks;
}

// Helper: Convert explicit blocks to canonical RGS
inline std::vector<std::size_t> blocks_to_rgs(const std::vector<std::vector<std::size_t>>& blocks, std::size_t n) {
    std::vector<std::size_t> rgs(n, 0);
    // Sort blocks by their minimum element to ensure canonical RGS labeling
    std::vector<std::vector<std::size_t>> sorted_blocks = blocks;
    for (auto& b : sorted_blocks) {
        std::sort(b.begin(), b.end());
    }
    std::sort(sorted_blocks.begin(), sorted_blocks.end(), [](const auto& a, const auto& b) {
        if (a.empty()) return false;
        if (b.empty()) return true;
        return a[0] < b[0];
    });

    for (std::size_t b_idx = 0; b_idx < sorted_blocks.size(); ++b_idx) {
        for (std::size_t elem : sorted_blocks[b_idx]) {
            if (elem < n) {
                rgs[elem] = b_idx;
            }
        }
    }
    return rgs;
}

// View generating all set partitions of {0, ..., n-1} as canonical RGS in lexicographical order.
// Total partitions generated: Bell number B_n.
class set_partitions_view {
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
                a_.assign(n, 0);
                m_.assign(n, 0);
            }
        }

        reference operator*() const noexcept {
            return std::span<const std::size_t>(a_.data(), a_.size());
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
            return a_ == other.a_;
        }

        bool operator!=(const iterator& other) const noexcept {
            return !(*this == other);
        }

    private:
        void advance() {
            if (done_ || n_ <= 1) {
                done_ = true;
                return;
            }

            // Knuth Algorithm H for restricted growth strings
            // Find largest index j > 0 where a_[j] <= m_[j-1]
            std::size_t j = n_ - 1;
            while (j > 0 && a_[j] > m_[j - 1]) {
                --j;
            }

            if (j == 0) {
                done_ = true;
                return;
            }

            ++a_[j];
            m_[j] = std::max(m_[j - 1], a_[j]);

            // Reset tail
            for (std::size_t i = j + 1; i < n_; ++i) {
                a_[i] = 0;
                m_[i] = m_[j];
            }
        }

        std::size_t n_{0};
        bool done_{false};
        std::vector<std::size_t> a_;
        std::vector<std::size_t> m_;
    };

    explicit set_partitions_view(std::size_t n) : n_(n) {}

    iterator begin() const { return iterator(n_); }
    iterator end() const { return iterator(); }

    std::size_t size_elements() const noexcept { return n_; }

private:
    std::size_t n_;
};

// View generating all set partitions of {0, ..., n-1} into exactly k non-empty blocks.
// Total partitions generated: Stirling number of the second kind S(n, k).
class k_set_partitions_view {
public:
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::span<const std::size_t>;
        using difference_type = std::ptrdiff_t;
        using pointer = const std::span<const std::size_t>*;
        using reference = std::span<const std::size_t>;

        iterator() : done_(true) {}

        iterator(std::size_t n, std::size_t k)
            : n_(n), k_(k), done_(k == 0 || k > n) {
            if (!done_) {
                a_.assign(n, 0);
                m_.assign(n, 0);

                // Initial lexicographically smallest RGS with exactly k blocks:
                // [0, 0, ..., 0, 1, 2, ..., k-1]
                std::size_t prefix_zeros = n - k + 1;
                for (std::size_t i = prefix_zeros; i < n; ++i) {
                    a_[i] = i - prefix_zeros + 1;
                }
                for (std::size_t i = 0; i < n; ++i) {
                    m_[i] = (i == 0) ? a_[0] : std::max(m_[i - 1], a_[i]);
                }
            }
        }

        reference operator*() const noexcept {
            return std::span<const std::size_t>(a_.data(), a_.size());
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
            return a_ == other.a_;
        }

        bool operator!=(const iterator& other) const noexcept {
            return !(*this == other);
        }

    private:
        void advance() {
            if (done_ || n_ == 0 || k_ == 0 || k_ > n_) {
                done_ = true;
                return;
            }

            // Backtrack from right to find index j where a_[j] can be incremented
            // Conditions:
            // 1. a_[j] + 1 <= m_[j-1] + 1
            // 2. a_[j] + 1 <= k_ - 1
            // 3. New running max + remaining slots >= k_ - 1
            std::size_t j = n_ - 1;
            while (j > 0) {
                std::size_t max_allowed = std::min(m_[j - 1] + 1, k_ - 1);
                if (a_[j] < max_allowed) {
                    std::size_t candidate = a_[j] + 1;
                    std::size_t new_m = std::max(m_[j - 1], candidate);
                    std::size_t remaining_slots = n_ - 1 - j;
                    if (new_m + remaining_slots >= k_ - 1) {
                        a_[j] = candidate;
                        m_[j] = new_m;
                        break;
                    }
                }
                --j;
            }

            if (j == 0) {
                done_ = true;
                return;
            }

            // Fill tail j + 1 .. n - 1 with minimal valid values
            for (std::size_t i = j + 1; i < n_; ++i) {
                std::size_t remaining_slots = n_ - 1 - i;
                std::size_t needed_increases = (k_ - 1 > m_[i - 1]) ? ((k_ - 1) - m_[i - 1]) : 0;

                if (remaining_slots + 1 == needed_increases) {
                    // Must increase running max to ensure reaching k_ - 1
                    a_[i] = m_[i - 1] + 1;
                } else {
                    a_[i] = 0;
                }
                m_[i] = std::max(m_[i - 1], a_[i]);
            }
        }

        std::size_t n_{0};
        std::size_t k_{0};
        bool done_{false};
        std::vector<std::size_t> a_;
        std::vector<std::size_t> m_;
    };

    k_set_partitions_view(std::size_t n, std::size_t k) : n_(n), k_(k) {}

    iterator begin() const { return iterator(n_, k_); }
    iterator end() const { return iterator(); }

    std::size_t size_elements() const noexcept { return n_; }
    std::size_t block_target() const noexcept { return k_; }

private:
    std::size_t n_;
    std::size_t k_;
};

} // namespace discretex::combinatorics
