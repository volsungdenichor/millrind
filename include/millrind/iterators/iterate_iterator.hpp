#pragma once

#include "../iterator_facade.hpp"
#include "default_constructible_func.hpp"

namespace millrind
{
template<class Iter>
class iterate_iterator : public iterator_facade<iterate_iterator<Iter>>
{
public:
    iterate_iterator() = default;

    iterate_iterator(Iter iter)
        : _iter{ std::move(iter) }
    {
    }

    iterate_iterator(const iterate_iterator&) = default;

    auto deref() const
    {
        return _iter;
    }

    void inc()
    {
        ++_iter;
    }

    template<class It = Iter, class = bidirectional_iterator<It>>
    void dec()
    {
        --_iter;
    }

    bool is_equal(const iterate_iterator& other) const
    {
        return _iter == other._iter;
    }

    template<class It = Iter, class = random_access_iterator<It>>
    bool is_less(const iterate_iterator& other) const
    {
        return _iter < other._iter;
    }

    template<class It = Iter, class = random_access_iterator<It>>
    void advance(std::ptrdiff_t offset)
    {
        _iter += offset;
    }

    template<class It = Iter, class = random_access_iterator<It>>
    auto distance_to(const iterate_iterator& other) const
    {
        return other._iter - _iter;
    }

private:
    Iter _iter;
};

}  // namespace millrind

MILLRIND_ITERATOR_TRAITS(::millrind::iterate_iterator)