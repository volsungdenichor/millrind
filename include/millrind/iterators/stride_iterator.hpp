#pragma once

#include "../iterator_facade.hpp"
#include "../iterator_range.hpp"

namespace millrind
{
template<class Iter>
class stride_iterator : public iterator_facade<stride_iterator<Iter>>
{
public:
    stride_iterator() = default;

    stride_iterator(Iter iter, std::ptrdiff_t step, Iter last)
        : _iter{ iter }
        , _step{ step }
        , _last{ last }
    {
    }

    stride_iterator(const stride_iterator&) = default;

    stride_iterator(stride_iterator&&) = default;

    decltype(auto) deref() const
    {
        return *_iter;
    }

    void inc()
    {
        _iter = ::millrind::advance(_iter, _step, _last);
    }

    bool is_equal(const stride_iterator& other) const
    {
        return _iter == other._iter;
    }

private:
    Iter _iter;
    std::ptrdiff_t _step;
    Iter _last;
};

}  // namespace millrind

MILLRIND_ITERATOR_TRAITS(::millrind::stride_iterator)