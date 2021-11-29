#pragma once

#include <cstddef>
#include <utility>

namespace millrind
{
namespace detail
{
struct dereference_fn
{
    template <class T>
    constexpr decltype(auto) operator()(T&& item) const
    {
        return *std::forward<T>(item);
    }
};

template <size_t I>
struct element_fn
{
    template <class T>
    constexpr decltype(auto) operator()(T&& item) const
    {
        return std::get<I>(std::forward<T>(item));
    }
};

}  // namespace detail

static constexpr inline auto dereference = detail::dereference_fn{};

template <size_t I>
static constexpr inline auto element = detail::element_fn<I>{};
}  // namespace millrind
