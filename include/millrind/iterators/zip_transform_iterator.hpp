#pragma once

#include "../iterator_facade.hpp"
#include "default_constructible_func.hpp"

namespace millrind
{
template <class Func, class... Iters>
class zip_transform_iterator : public iterator_facade<zip_transform_iterator<Func, Iters...>>
{
private:
    using index_seq = std::index_sequence_for<Iters...>;

public:
    zip_transform_iterator() = default;

    zip_transform_iterator(Func func, Iters... iters)
        : _func{ std::move(func) }
        , _iters{ std::move(iters)... }
    {
    }

    zip_transform_iterator(const zip_transform_iterator&) = default;

    decltype(auto) deref() const
    {
        return deref(index_seq{});
    }

    void inc()
    {
        inc(index_seq{});
    }

    void dec()
    {
        dec(index_seq{});
    }

    bool is_equal(const zip_transform_iterator& other) const
    {
        return std::get<0>(_iters) == std::get<0>(other._iters);
    }

private:
    template <size_t... I>
    decltype(auto) deref(std::index_sequence<I...>) const
    {
        return call(_func, *std::get<I>(_iters)...);
    }

    template <size_t... I>
    void inc(std::index_sequence<I...>)
    {
        ignore(++std::get<I>(_iters)...);
    }

    template <size_t... I>
    void dec(std::index_sequence<I...>)
    {
        ignore(--std::get<I>(_iters)...);
    }

    template <class... T>
    void ignore(T&&...)
    {
    }

    default_constructible_func<Func> _func;
    std::tuple<Iters...> _iters;
};

}  // namespace millrind

MILLRIND_ITERATOR_TRAITS(::millrind::zip_transform_iterator)