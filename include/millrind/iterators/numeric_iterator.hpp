#pragma once

#include "../iterator_facade.hpp"

namespace millrind
{
template <class T>
class numeric_iterator : public iterator_facade<numeric_iterator<T>>
{
public:
    numeric_iterator() = default;

    numeric_iterator(T value)
        : _value{ std::move(value) }
    {
    }

    numeric_iterator(const numeric_iterator&) = default;

    numeric_iterator& operator=(numeric_iterator other)
    {
        std::swap(_value, other._value);
        return *this;
    }

    auto deref() const
    {
        return _value;
    }

    void advance(std::ptrdiff_t offset)
    {
        _value += static_cast<T>(offset);
    }

    auto distance_to(const numeric_iterator& other) const
    {
        return other._value - _value;
    }

private:
    T _value;
};

}  // namespace millrind

MILLRIND_ITERATOR_TRAITS(::millrind::numeric_iterator)