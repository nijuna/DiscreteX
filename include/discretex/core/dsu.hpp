#pragma once

#include <cstddef>
#include <vector>
#include <numeric>
#include <algorithm>
#include <utility>
#include <stdexcept>

namespace discretex {

// Disjoint Set Union (DSU) / Union-Find data structure over index domain [0, n).
// Uses union by size and path compression to achieve amortized O(alpha(n)) operations.
class disjoint_set {
public:
    explicit disjoint_set(std::size_t n = 0)
        : parent_(n), size_(n, 1), num_sets_(n) {
        std::iota(parent_.begin(), parent_.end(), std::size_t{0});
    }

    // Finds the canonical representative of element x with path compression.
    // Marked const because path compression is an internal tree flattening
    // optimization that preserves the logical equivalence relation invariant.
    [[nodiscard]] std::size_t find(std::size_t x) const {
        if (x >= parent_.size()) {
            throw std::out_of_range("Element index out of bounds in disjoint_set::find");
        }
        std::size_t root = x;
        while (root != parent_[root]) {
            root = parent_[root];
        }
        // Path compression
        std::size_t curr = x;
        while (curr != root) {
            std::size_t next = parent_[curr];
            parent_[curr] = root;
            curr = next;
        }
        return root;
    }

    // Unites the sets containing elements x and y using union by size.
    // Returns true if x and y were previously in different sets, false if already united.
    bool unite(std::size_t x, std::size_t y) {
        std::size_t root_x = find(x);
        std::size_t root_y = find(y);

        if (root_x == root_y) {
            return false;
        }

        // Union by size: attach smaller tree under larger tree
        if (size_[root_x] < size_[root_y]) {
            std::swap(root_x, root_y);
        }

        parent_[root_y] = root_x;
        size_[root_x] += size_[root_y];
        --num_sets_;
        return true;
    }

    // Checks if x and y belong to the same equivalence class.
    [[nodiscard]] bool connected(std::size_t x, std::size_t y) const {
        return find(x) == find(y);
    }

    // Total number of elements in the domain [0, n).
    [[nodiscard]] std::size_t size() const noexcept {
        return parent_.size();
    }

    // Number of disjoint sets (connected components).
    [[nodiscard]] std::size_t component_count() const noexcept {
        return num_sets_;
    }

    // Number of elements in the set containing x.
    [[nodiscard]] std::size_t component_size(std::size_t x) const {
        return size_[find(x)];
    }

    // Extracts all disjoint components as lists of vertex indices.
    [[nodiscard]] std::vector<std::vector<std::size_t>> components() const {
        std::size_t n = parent_.size();
        std::vector<std::vector<std::size_t>> comp_map(n);
        for (std::size_t i = 0; i < n; ++i) {
            comp_map[find(i)].push_back(i);
        }

        std::vector<std::vector<std::size_t>> result;
        result.reserve(num_sets_);
        for (std::size_t i = 0; i < n; ++i) {
            if (!comp_map[i].empty()) {
                result.push_back(std::move(comp_map[i]));
            }
        }
        return result;
    }

    // Generates canonical Restricted Growth String (RGS) for the partition.
    [[nodiscard]] std::vector<std::size_t> to_rgs() const {
        std::size_t n = parent_.size();
        if (n == 0) return {};

        std::vector<std::size_t> rgs(n, n);
        std::size_t current_block = 0;

        for (std::size_t u = 0; u < n; ++u) {
            if (rgs[u] == n) {
                std::size_t rep = find(u);
                rgs[u] = current_block;
                for (std::size_t v = u + 1; v < n; ++v) {
                    if (find(v) == rep) {
                        rgs[v] = current_block;
                    }
                }
                ++current_block;
            }
        }
        return rgs;
    }

private:
    mutable std::vector<std::size_t> parent_;
    std::vector<std::size_t> size_;
    std::size_t num_sets_{0};
};

// Canonical type alias
using dsu = disjoint_set;

} // namespace discretex
