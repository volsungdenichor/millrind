#pragma once

#include "../iterator_facade.hpp"

namespace millrind
{
template<class Iter1, class Iter2>
class chain_iterator : public iterator_facade<chain_iterator<Iter1, Iter2>>
{
public:
    chain_iterator() = default;

    chain_iterator(Iter1 iter1, Iter2 iter2, Iter1 end, Iter2 begin)
        : _iter1{ iter1 },
          _iter2{ iter2 },
          _range1_end{ end },
          _range2_begin{ begin }
    {
    }

    chain_iterator(const chain_iterator&) = default;

    decltype(auto) deref() const
    {
        return _iter1 != _range1_end
                   ? *_iter1
                   : *_iter2;
    }

    void inc()
    {
        if (_iter1 != _range1_end)
            ++_iter1;
        else
            ++_iter2;
    }

    template<class It1 = Iter1, class It2 = Iter2, class = bidirectional_iterator<It1>, class = bidirectional_iterator<It2>>
    void dec()
    {
        if (_iter2 != _range2_begin)
            --_iter2;
        else
            --_iter1;
    }

    auto is_equal(const chain_iterator& other) const
    {
        return _iter1 == other._iter1 && _iter2 == other._iter2;
    }

private:
    Iter1 _iter1;
    Iter2 _iter2;

    Iter1 _range1_end;
    Iter2 _range2_begin;
};

}  // namespace millrind

MILLRIND_ITERATOR_TRAITS(::millrind::chain_iterator)