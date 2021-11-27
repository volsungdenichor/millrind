#pragma once

#include <utility>

#include "type_traits.hpp"

namespace millrind
{
template<class Iter, class Diff>
constexpr Iter advance(Iter it, Diff count, Iter end)
{
    if constexpr (is_detected_v<random_access_iterator, Iter>)
    {
        return std::next(it, std::min<Diff>(std::distance(it, end), count));
    }
    else
    {
        while (it != end && count > 0)
        {
            ++it;
            --count;
        }
        return it;
    }
}

template<bool Expected = true, class Iter, class Pred>
constexpr Iter advance_while(Iter it, Pred pred, Iter end)
{
    while (it != end && (std::invoke(pred, *it) == Expected))
    {
        ++it;
    }
    return it;
}

template<class Iter>
class iterator_range
{
private:
    using traits = std::iterator_traits<Iter>;

public:
    using iterator = Iter;
    using reference = typename traits::reference;
    using difference_type = typename traits::difference_type;
    using size_type = difference_type;
    using value_type = typename traits::value_type;
    using iterator_category = typename traits::iterator_category;

    constexpr iterator_range(iterator begin, iterator end)
        : begin_{ begin },
          end_{ end }
    {
    }

    constexpr iterator_range(std::pair<iterator, iterator> iterators)
        : iterator_range{ std::get<0>(iterators), std::get<1>(iterators) }
    {
    }

    constexpr iterator_range()
        : begin_{},
          end_{}
    {
    }

    constexpr iterator_range(const iterator_range&) = default;

    constexpr iterator_range(iterator_range&&) = default;

    constexpr iterator_range& operator=(iterator_range other)
    {
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        return *this;
    }

    template<class Container, class = std::enable_if_t<std::is_constructible_v<Container, iterator, iterator>>>
    operator Container() const
    {
        return { begin(), end() };
    }

    constexpr iterator begin() const
    {
        return begin_;
    }

    constexpr iterator end() const
    {
        return end_;
    }

    constexpr bool empty() const
    {
        return begin() == end();
    }

    constexpr size_type size() const
    {
        return std::distance(begin(), end());
    }

    template<size_t Index>
    constexpr iterator get() const
    {
        if constexpr (Index == 0)
            return begin();
        else if constexpr (Index == 1)
            return end();
    }

    constexpr reference front() const
    {
        return *begin();
    }

    template<class It = iterator, class = bidirectional_iterator<It>>
    constexpr reference back() const
    {
        return *std::prev(end());
    }

#if 0
    constexpr explicit operator bool() const
    {
        return !empty();
    }

    constexpr reference operator*() const
    {
        return front();
    }
#endif

    template<class It = iterator, class = random_access_iterator<It>>
    constexpr reference operator[](difference_type offset) const
    {
        return *std::next(begin(), offset);
    }

private:
    iterator begin_;
    iterator end_;
};

namespace detail
{
struct make_range_fn
{
    template<class Iter>
    constexpr auto operator()(Iter begin, Iter end) const -> iterator_range<Iter>
    {
        return { std::move(begin), std::move(end) };
    }

    template<class Iter>
    constexpr auto operator()(std::pair<Iter, Iter> p) const -> iterator_range<Iter>
    {
        auto [b, e] = p;
        return (*this)(b, e);
    }

    template<class Range>
    constexpr auto operator()(Range&& range) const
    {
        return (*this)(std::begin(range), std::end(range));
    }
};
}  // namespace detail

static constexpr inline auto make_range = detail::make_range_fn{};

}  // namespace millrind

namespace std
{
template<class Iter>
struct tuple_size<::millrind::iterator_range<Iter>> : std::integral_constant<size_t, 2>
{
};

template<size_t Index, class Iter>
struct tuple_element<Index, ::millrind::iterator_range<Iter>>
{
    using type = Iter;
};

} /* namespace std */