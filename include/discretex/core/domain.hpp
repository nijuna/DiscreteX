#pragma once
#include <concepts>
#include <cstddef>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <ranges>

namespace discretex {

// 1. Basic finite domain with stable index-to-element mapping
template <typename D>
concept FiniteDomain = requires(const D& d, std::size_t i) {
    typename D::value_type;
    { d.size() } -> std::convertible_to<std::size_t>;
    { d.from_index(i) } -> std::convertible_to<typename D::value_type>;
};

// 2. Stronger domain supporting element-to-index mapping
template <typename D>
concept IndexableDomain = FiniteDomain<D> && requires(const D& d, const typename D::value_type& x) {
    { d.to_index(x) } -> std::convertible_to<std::size_t>;
    { d.contains(x) } -> std::convertible_to<bool>;
};

// Pure contiguous integer domain [0, n)
class index_domain {
public:
    using value_type = std::size_t;

    constexpr explicit index_domain(std::size_t n = 0) noexcept : size_(n) {}

    [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }
    [[nodiscard]] constexpr std::size_t to_index(std::size_t x) const noexcept { return x; }
    [[nodiscard]] constexpr std::size_t from_index(std::size_t i) const noexcept { return i; }
    [[nodiscard]] constexpr bool contains(std::size_t x) const noexcept { return x < size_; }

    auto elements() const noexcept {
        return std::views::iota(std::size_t{0}, size_);
    }

private:
    std::size_t size_{0};
};

// Mapped domain for arbitrary discrete types T
template <typename T, typename Hash = std::hash<T>, typename KeyEqual = std::equal_to<T>>
class mapped_domain {
public:
    using value_type = T;

    mapped_domain() = default;

    mapped_domain(std::initializer_list<T> items) {
        for (const auto& item : items) add(item);
    }

    template <std::ranges::input_range R>
    explicit mapped_domain(R&& range) {
        for (auto&& item : range) add(item);
    }

    std::size_t add(const T& val) {
        auto [it, inserted] = map_.emplace(val, elements_.size());
        if (inserted) elements_.push_back(val);
        return it->second;
    }

    [[nodiscard]] std::size_t size() const noexcept { return elements_.size(); }

    [[nodiscard]] std::size_t to_index(const T& val) const {
        auto it = map_.find(val);
        if (it == map_.end()) throw std::out_of_range("Element not in domain");
        return it->second;
    }

    [[nodiscard]] const T& from_index(std::size_t i) const {
        return elements_.at(i);
    }

    [[nodiscard]] bool contains(const T& val) const noexcept {
        return map_.contains(val);
    }

    [[nodiscard]] const std::vector<T>& elements() const noexcept {
        return elements_;
    }

private:
    std::vector<T> elements_;
    std::unordered_map<T, std::size_t, Hash, KeyEqual> map_;
};

static_assert(IndexableDomain<index_domain>);
static_assert(IndexableDomain<mapped_domain<std::string>>);

} // namespace discretex
