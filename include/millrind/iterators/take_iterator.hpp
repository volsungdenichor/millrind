#pragma once

#include <optional>

#include "../iterator_facade.hpp"

namespace millrind
{
template<class Iter>
class take_iterator : public iterator_facade<take_iterator<Iter>>
{
public:
    take_iterator() = default;

    take_iterator(Iter iter, std::ptrdiff_t count)
        : _iter{ std::move(iter) },
          _count{ count }
    {
    }

    take_iterator(Iter iter)
        : _iter{ std::move(iter) },
          _count{}
    {
    }

    take_iterator(const take_iterator&) = default;

    auto deref() const -> iter_reference_t<Iter>
    {
        return *_iter;
    }

    void inc()
    {
        ++_iter;
        --*_count;
    }

    bool is_equal(const take_iterator& other) const
    {
        return _iter == other._iter || (_count && *_count <= 0);
    }

private:
    Iter _iter;
    std::optional<std::ptrdiff_t> _count;
};

}  // namespace millrind

MILLRIND_ITERATOR_TRAITS(::millrind::take_iterator)