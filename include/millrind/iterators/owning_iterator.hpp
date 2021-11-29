#pragma once

#include <memory>

#include "../iterator_facade.hpp"

namespace millrind
{
template <class Iter, class Container>
class owning_iterator : public iterator_facade<owning_iterator<Iter, Container>>
{
public:
    owning_iterator() = default;

    owning_iterator(Iter iter, std::shared_ptr<Container> container)
        : _iter{ iter }
        , _container{ std::move(container) }
    {
    }

    owning_iterator(const owning_iterator&) = default;

    owning_iterator& operator=(owning_iterator other)
    {
        std::swap(_iter, other._iter);
        std::swap(_container, other._container);
        return *this;
    }

    decltype(auto) deref() const
    {
        return *_iter;
    }

    void inc()
    {
        ++_iter;
    }

    template <class It = Iter, class = bidirectional_iterator<It>>
    void dec()
    {
        --_iter;
    }

    bool is_equal(const owning_iterator& other) const
    {
        return _iter == other._iter;
    }

    template <class It = Iter, class = random_access_iterator<It>>
    bool is_less(const owning_iterator& other) const
    {
        return _iter < other._iter;
    }

    template <class It = Iter, class = random_access_iterator<It>>
    void advance(std::ptrdiff_t offset)
    {
        _iter += offset;
    }

    template <class It = Iter, class = random_access_iterator<It>>
    auto distance_to(const owning_iterator& other) const
    {
        return other._iter - _iter;
    }

private:
    Iter _iter;
    std::shared_ptr<Container> _container;
};

}  // namespace millrind

MILLRIND_ITERATOR_TRAITS(::millrind::owning_iterator)