#pragma once

#include "../iterator_facade.hpp"
#include "../opt.hpp"
#include "default_constructible_func.hpp"

namespace millrind
{
template<class Iter>
class cached_iterator : public iterator_facade<cached_iterator<Iter>>
{
public:
    cached_iterator() = default;

    cached_iterator(Iter iter)
        : _iter{ std::move(iter) },
          _cache{}
    {
    }

    cached_iterator(const cached_iterator&) = default;

    decltype(auto) deref() const
    {
        if (!_cache)
        {
            _cache = wrap(*_iter);
        }
        return unwrap(*_cache);
    }

    void inc()
    {
        invalidate();
        ++_iter;
    }

    template<class It = Iter, class = bidirectional_iterator<It>>
    void dec()
    {
        invalidate();
        --_iter;
    }

    bool is_equal(const cached_iterator& other) const
    {
        return _iter == other._iter;
    }

    template<class It = Iter, class = random_access_iterator<It>>
    bool is_less(const cached_iterator& other) const
    {
        return _iter < other._iter;
    }

    template<class It = Iter, class = random_access_iterator<It>>
    void advance(std::ptrdiff_t offset)
    {
        invalidate();
        _iter += offset;
    }

    template<class It = Iter, class = random_access_iterator<It>>
    auto distance_to(const cached_iterator& other) const
    {
        return other._iter - _iter;
    }

private:
    void invalidate()
    {
        _cache = {};
    }

    Iter _iter;
    mutable std::optional<wrapped<iter_reference_t<Iter>>> _cache;
};  // namespace millrind

}  // namespace millrind

MILLRIND_ITERATOR_TRAITS(::millrind::cached_iterator)