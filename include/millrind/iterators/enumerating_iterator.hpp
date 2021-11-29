#pragma once

#include "../iterator_facade.hpp"

namespace millrind
{
template <class Iter>
class enumerating_iterator : public iterator_facade<enumerating_iterator<Iter>>
{
public:
    enumerating_iterator() = default;

    enumerating_iterator(Iter iter, std::ptrdiff_t index)
        : _iter{ std::move(iter) }
        , _index{ index }
    {
    }

    enumerating_iterator(Iter iter)
        : _iter{ std::move(iter) }
        , _index{}
    {
    }

    enumerating_iterator(const enumerating_iterator&) = default;

    auto deref() const -> std::pair<std::ptrdiff_t, iter_reference_t<Iter>>
    {
        return { *_index, *_iter };
    }

    void inc()
    {
        ++_iter;
        ++*_index;
    }

    template <class It = Iter, class = bidirectional_iterator<It>>
    void dec()
    {
        --_iter;
        --*_index;
    }

    bool is_equal(const enumerating_iterator& other) const
    {
        return _iter == other._iter;
    }

    template <class It = Iter, class = random_access_iterator<It>>
    bool is_less(const enumerating_iterator& other) const
    {
        return _iter < other._iter;
    }

    template <class It = Iter, class = random_access_iterator<It>>
    void advance(std::ptrdiff_t offset)
    {
        _iter += offset;
        *_index += offset;
    }

    template <class It = Iter, class = random_access_iterator<It>>
    auto distance_to(const enumerating_iterator& other) const
    {
        return other._iter - _iter;
    }

private:
    Iter _iter;
    std::optional<std::ptrdiff_t> _index;
};

}  // namespace millrind

MILLRIND_ITERATOR_TRAITS(::millrind::enumerating_iterator)