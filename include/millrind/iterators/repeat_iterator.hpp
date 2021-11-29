#pragma once

#include "../iterator_facade.hpp"
#include "default_constructible_func.hpp"

namespace millrind
{
template<class T>
class repeat_iterator : public iterator_facade<repeat_iterator<T>>
{
public:
    repeat_iterator() = default;

    repeat_iterator(T value, std::ptrdiff_t index)
        : _value{ std::move(value) }
        , _index{ index }
    {
    }

    repeat_iterator(const repeat_iterator&) = default;

    decltype(auto) deref() const
    {
        return _value;
    }

    void advance(std::ptrdiff_t offset)
    {
        _index += offset;
    }

    auto distance_to(const repeat_iterator& other) const
    {
        return other._index - _index;
    }

private:
    T _value;
    std::ptrdiff_t _index;
};

}  // namespace millrind

MILLRIND_ITERATOR_TRAITS(::millrind::repeat_iterator)