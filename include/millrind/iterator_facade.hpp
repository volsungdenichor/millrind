#pragma once

#include <iterator>

#include "type_traits.hpp"

namespace millrind
{
namespace detail
{
template <class T>
struct pointer_proxy
{
    T item;

    T* operator->()
    {
        return &item;
    }
};

}  // namespace detail

template <class T>
using has_deref = decltype(std::declval<T>().deref());

template <class T>
using has_inc = decltype(std::declval<T>().inc());

template <class T>
using has_dec = decltype(std::declval<T>().dec());

template <class T>
using has_advance = decltype(std::declval<T>().advance(std::declval<convertible_to<std::is_integral>>()));

template <class T>
using has_is_equal = decltype(std::declval<T>().is_equal(std::declval<T>()));

template <class T>
using has_is_less = decltype(std::declval<T>().is_less(std::declval<T>()));

template <class T>
using has_distance_to = decltype(std::declval<T>().distance_to(std::declval<T>()));

template <class T>
static constexpr bool has_deref_v = is_detected_v<has_deref, T>;

template <class T>
static constexpr bool has_inc_v = is_detected_v<has_inc, T>;

template <class T>
static constexpr bool has_dec_v = is_detected_v<has_dec, T>;

template <class T>
static constexpr bool has_advance_v = is_detected_v<has_advance, T>;

template <class T>
static constexpr bool has_is_equal_v = is_detected_v<has_is_equal, T>;

template <class T>
static constexpr bool has_is_less_v = is_detected_v<has_is_less, T>;

template <class T>
static constexpr bool has_distance_to_v = is_detected_v<has_distance_to, T>;

template <class Self>
class iterator_facade
{
protected:
    using self_type = Self;

public:
    self_type& operator=(self_type other)
    {
        std::swap(self(), other);
        return self();
    }

    decltype(auto) operator*() const
    {
        static_assert(has_deref_v<self_type>, "iterator_facade: deref() method required");

        return self().deref();
    }

    auto operator->() const
    {
        decltype(auto) ref = **this;
        if constexpr (std::is_reference_v<decltype(ref)>)
            return std::addressof(ref);
        else
            return detail::pointer_proxy<decltype(ref)>{ std::move(ref) };
    }

    self_type& operator++()
    {
        if constexpr (has_inc_v<self_type>)
        {
            self().inc();
        }
        else
        {
            static_assert(has_advance_v<self_type>, "iterator_facade: either inc() or advance() methods required");

            self() += 1;
        }
        return self();
    }

    self_type operator++(int)
    {
        auto temp = self();
        ++self();
        return temp;
    }

    self_type& operator--()
    {
        if constexpr (has_dec_v<self_type>)
        {
            self().dec();
        }
        else
        {
            static_assert(has_advance_v<self_type>, "iterator_facade: either dec() or advance() methods required");

            self() -= 1;
        }
        return self();
    }

    self_type operator--(int)
    {
        auto temp = self();
        --self();
        return temp;
    }

    template <class D>
    friend self_type& operator+=(self_type& it, D offset)
    {
        static_assert(has_advance_v<self_type>, "iterator_facade: advance() methods required");

        it.advance(offset);
        return it;
    }

    template <class D>
    friend self_type operator+(self_type it, D offset)
    {
        return it += offset;
    }

    template <class D>
    friend self_type& operator-=(self_type& it, D offset)
    {
        it += -offset;
        return it;
    }

    template <class D>
    friend self_type operator-(self_type it, D offset)
    {
        return it -= offset;
    }

    template <class D>
    decltype(auto) operator[](D offset) const
    {
        return *(self() + offset);
    }

    friend auto operator-(const self_type& lhs, const self_type& rhs)
    {
        static_assert(has_distance_to_v<self_type>, "iterator_facade: distance_to() method required");

        return rhs.distance_to(lhs);
    }

    friend bool operator==(const self_type& lhs, const self_type& rhs)
    {
        if constexpr (has_is_equal_v<self_type>)
        {
            return lhs.is_equal(rhs);
        }
        else
        {
            static_assert(
                has_distance_to_v<self_type>, "iterator_facade: either is_equal() or distance_to() method required");

            return lhs.distance_to(rhs) == 0;
        }
    }

    friend bool operator!=(const self_type& lhs, const self_type& rhs)
    {
        return !(lhs == rhs);
    }

    friend bool operator<(const self_type& lhs, const self_type& rhs)
    {
        if constexpr (has_is_less_v<self_type>)
        {
            return lhs.is_less(rhs);
        }
        else
        {
            static_assert(
                has_distance_to_v<self_type>, "iterator_facade: either is_less() or distance_to() method required");

            return lhs.distance_to(rhs) > 0;
        }
    }

    friend bool operator>(const self_type& lhs, const self_type& rhs)
    {
        return rhs < lhs;
    }

    friend bool operator<=(const self_type& lhs, const self_type& rhs)
    {
        return !(lhs > rhs);
    }

    friend bool operator>=(const self_type& lhs, const self_type& rhs)
    {
        return !(lhs < rhs);
    }

protected:
    const self_type& self() const
    {
        return static_cast<const self_type&>(*this);
    }

    self_type& self()
    {
        return static_cast<self_type&>(*this);
    }
};

namespace detail
{
template <class Iter, class = std::void_t<>>
struct difference_type_impl
{
    using type = std::ptrdiff_t;
};

template <class Iter>
struct difference_type_impl<Iter, std::void_t<has_distance_to<Iter>>>
{
    using type = decltype(std::declval<Iter>().distance_to(std::declval<Iter>()));
};

template <class Iter, class = std::void_t<>>
struct iterator_category_impl
{
    using type = std::forward_iterator_tag;
};

template <class Iter>
constexpr bool is_random_access = has_advance_v<Iter> && (has_distance_to_v<Iter> || has_is_less_v<Iter>);

template <class Iter>
constexpr bool is_bidirectional = (has_inc_v<Iter> && has_dec_v<Iter>) || has_advance_v<Iter>;

}  // namespace detail

template <class Iter>
struct iterator_traits
{
    using it = Iter;
    using reference = decltype(std::declval<it>().operator*());
    using pointer = decltype(std::declval<it>().operator->());
    using value_type = std::decay_t<reference>;
    using difference_type = typename detail::difference_type_impl<Iter>::type;
    using iterator_category = std::conditional_t<
        detail::is_random_access<Iter>,
        std::random_access_iterator_tag,
        std::conditional_t<detail::is_bidirectional<Iter>, std::bidirectional_iterator_tag, std::forward_iterator_tag>>;
};

}  // namespace millrind

#define MILLRIND_ITERATOR_TRAITS(iter)                                                 \
    namespace std                                                                      \
    {                                                                                  \
    template <class... Args>                                                           \
    struct iterator_traits<iter<Args...>> : ::millrind::iterator_traits<iter<Args...>> \
    {                                                                                  \
    };                                                                                 \
    }
