#pragma once

#include "../iterator_facade.hpp"
#include "../opt.hpp"
#include "default_constructible_func.hpp"

namespace millrind
{
template<class Iter>
class cache_latest_iterator : public iterator_facade<cache_latest_iterator<Iter>>
{
public:
    cache_latest_iterator() = default;

    cache_latest_iterator(Iter iter)
        : _iter{ std::move(iter) }
        , _cache{}
    {
    }

    cache_latest_iterator(const cache_latest_iterator&) = default;

    auto deref() const -> iter_reference_t<Iter>
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

    bool is_equal(const cache_latest_iterator& other) const
    {
        return _iter == other._iter;
    }

    template<class It = Iter, class = random_access_iterator<It>>
    bool is_less(const cache_latest_iterator& other) const
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
    auto distance_to(const cache_latest_iterator& other) const
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

MILLRIND_ITERATOR_TRAITS(::millrind::cache_latest_iterator)