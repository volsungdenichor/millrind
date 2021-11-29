#pragma once

#include <iterator>
#include <type_traits>

namespace millrind
{
template <class T>
using iterator_t = decltype(std::begin(std::declval<T>()));

template <class T>
using iter_category_t = typename std::iterator_traits<T>::iterator_category;

template <class T>
using iter_reference_t = typename std::iterator_traits<T>::reference;

template <class T>
using iter_value_t = typename std::iterator_traits<T>::value_type;

template <class T>
using iter_difference_t = typename std::iterator_traits<T>::difference_type;

template <class T>
using range_category_t = iter_category_t<iterator_t<T>>;

template <class T>
using range_reference_t = iter_reference_t<iterator_t<T>>;

template <class T>
using range_value_t = iter_value_t<iterator_t<T>>;

template <class T>
using range_difference_t = iter_difference_t<iterator_t<T>>;

struct any_type
{
    template <class T>
    operator T();
};

template <template <class> class C>
struct convertible_to
{
    template <class T, class = std::enable_if_t<C<T>::value>>
    operator T() const;
};

namespace detail
{
template <class AlwaysVoid, template <class...> class Op, class... Args>
struct detector_impl : std::false_type
{
};

template <template <class...> class Op, class... Args>
struct detector_impl<std::void_t<Op<Args...>>, Op, Args...> : std::true_type
{
};

template <class Category, class T>
using iterator_of_category = std::enable_if_t<std::is_base_of_v<Category, typename std::iterator_traits<T>::iterator_category>>;

}  // namespace detail

template <template <class...> class Op, class... Args>
struct is_detected : detail::detector_impl<std::void_t<>, Op, Args...>
{
};

template <template <class...> class Op, class... Args>
static constexpr bool is_detected_v = is_detected<Op, Args...>::value;

template <class T>
using output_iterator = std::void_t<std::void_t<decltype(*std::declval<T>()++ = std::declval<any_type>())>>;

template <class T>
using input_iterator = detail::iterator_of_category<std::input_iterator_tag, T>;

template <class T>
using forward_iterator = detail::iterator_of_category<std::forward_iterator_tag, T>;

template <class T>
using bidirectional_iterator = detail::iterator_of_category<std::bidirectional_iterator_tag, T>;

template <class T>
using random_access_iterator = detail::iterator_of_category<std::random_access_iterator_tag, T>;

template <class T>
using input_range = input_iterator<iterator_t<T>>;

template <class T>
using forward_range = forward_iterator<iterator_t<T>>;

template <class T>
using bidirectional_range = bidirectional_iterator<iterator_t<T>>;

template <class T>
using random_access_range = random_access_iterator<iterator_t<T>>;

template <class T, class U = T>
using equality_comparable = decltype(std::declval<T>() == std::declval<U>());

template <class T, class U = T>
using less_than_comparable = decltype(std::declval<T>() < std::declval<U>());

template <class T, class U = T>
using comparable = std::conjunction<equality_comparable<T, U>, less_than_comparable<T, U>>;

template <class T, class U>
using addable = decltype(std::declval<T>() + std::declval<U>());

template <class T, class U>
using subtractable = decltype(std::declval<T>() - std::declval<U>());

template <class T, class U>
using multipliable = decltype(std::declval<T>() * std::declval<U>());

template <class T, class U>
using dividable = decltype(std::declval<T>() / std::declval<U>());

template <class T>
using incrementable = decltype(++std::declval<T>());

template <class T>
using decrementable = decltype(--std::declval<T>());

template <class T, class U = T>
using add_assignable = decltype(std::declval<T>() += std::declval<U>());

template <class T, class U = T>
using subtract_assignable = decltype(std::declval<T>() -= std::declval<U>());

template <class T, class U = T>
using multiply_assignable = decltype(std::declval<T>() *= std::declval<U>());

template <class T, class U = T>
using divide_assignable = decltype(std::declval<T>() /= std::declval<U>());

}  // namespace millrind
