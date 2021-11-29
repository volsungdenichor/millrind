#pragma once

#include "../iterator_facade.hpp"
#include "default_constructible_func.hpp"

namespace millrind
{
template<class Func>
class generating_iterator : public iterator_facade<generating_iterator<Func>>
{
private:
    using optional_type = std::invoke_result_t<Func>;

public:
    generating_iterator()
        : _func{}
        , _current{}
        , _index{ std::numeric_limits<std::ptrdiff_t>::max() }
    {
    }

    generating_iterator(Func func)
        : _func{ std::move(func) }
        , _current{ std::invoke(_func) }
        , _index{ 0 }
    {
    }

    generating_iterator(const generating_iterator&) = default;

    decltype(auto) deref() const
    {
        return unwrap(*_current);
    }

    void inc()
    {
        _current = std::invoke(_func);
        ++_index;
    }

    bool is_equal(const generating_iterator& other) const
    {
        return !_current || _index == other._index;
    }

private:
    default_constructible_func<Func> _func;
    std::ptrdiff_t _index;
    optional_type _current;
};

}  // namespace millrind

MILLRIND_ITERATOR_TRAITS(::millrind::generating_iterator)