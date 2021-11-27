#pragma once

#include <optional>
#include <type_traits>

#include "pipeable.hpp"

namespace millrind
{
namespace detail
{
template<class T, class = std::void_t<>>
struct is_optional : std::false_type
{
};

template<class T>
struct is_optional<T, std::void_t<decltype(static_cast<bool>(std::declval<T>())), decltype(*std::declval<T>())>> : std::true_type
{
};

template<class T>
constexpr auto to_optional(T&& item)
{
    return item ? std::optional{ wrap(*std::forward<T>(item)) }
                : std::nullopt;
}

struct some_fn
{
    template<class T>
    constexpr auto operator()(T value) const -> std::optional<T>
    {
        return { std::move(value) };
    }
};

struct has_value_fn
{
    template<class T>
    constexpr bool operator()(T&& item) const
    {
        static_assert(is_optional<std::remove_reference_t<T>>{}, "opt::has_value: optional type required");
        return static_cast<bool>(item);
    }
};

struct map_fn
{
    template<class T, class Func>
    constexpr auto operator()(T&& item, Func&& func) const
    {
        static_assert(is_optional<std::remove_reference_t<T>>{}, "opt::map: optional type required");
        return item ? std::optional{ wrap(call(std::forward<Func>(func), *std::forward<T>(item))) } : std::nullopt;
    }
};

struct flat_map_fn
{
    template<class T, class Func>
    constexpr auto operator()(T&& item, Func&& func) const
    {
        static_assert(is_optional<std::remove_reference_t<T>>{}, "opt::flat_map: optional type required");
        return item ? to_optional(wrap(call(std::forward<Func>(func), *std::forward<T>(item)))) : std::nullopt;
    }
};

template<bool Expected>
struct filter_fn
{
    template<class T, class Pred>
    constexpr auto operator()(T&& item, Pred&& pred) const
    {
        static_assert(is_optional<std::remove_reference_t<T>>{}, "opt::filter: optional type required");
        return (item && call(std::forward<Pred>(pred), *item) == Expected) ? to_optional(std::forward<T>(item)) : std::nullopt;
    }
};

struct value_fn
{
    template<class T>
    constexpr decltype(auto) operator()(T&& item) const
    {
        static_assert(is_optional<std::remove_reference_t<T>>{}, "opt::value: optional type required");
        if (!item)
            throw std::bad_optional_access{};
        return unwrap(*std::forward<T>(item));
    }
};

template<class T>
constexpr decltype(auto) get_or_invoke(T&& item_or_func)
{
    if constexpr (std::is_invocable_v<T>)
        return std::invoke(std::forward<T>(item_or_func));
    else
        return std::forward<T>(item_or_func);
}

struct value_or_fn
{
    template<class T, class U>
    constexpr decltype(auto) operator()(T&& item, U&& default_value) const
    {
        static_assert(is_optional<std::remove_reference_t<T>>{}, "opt::value_or: optional type required");
        return item ? unwrap(*std::forward<T>(item)) : unwrap(get_or_invoke(std::forward<U>(default_value)));
    }
};

struct value_or_throw_fn
{
    template<class T, class E, class = std::enable_if_t<std::is_base_of_v<std::exception, E>>>
    constexpr decltype(auto) operator()(T&& item, const E& exception) const
    {
        static_assert(is_optional<std::remove_reference_t<T>>{}, "opt::value_or_throw: optional type required");
        return item ? unwrap(*std::forward<T>(item)) : throw exception;
    }

    template<class T>
    constexpr decltype(auto) operator()(T&& item, const std::string& message) const
    {
        static_assert(is_optional<std::remove_reference_t<T>>{}, "opt::value_or_throw: optional type required");
        return item ? unwrap(*std::forward<T>(item)) : throw std::runtime_error{ message };
    }
};

struct disjunction_fn
{
    template<class T, class U>
    constexpr auto operator()(T&& item, U&& other) const
    {
        static_assert(is_optional<std::remove_reference_t<T>>{}, "opt::disjunction: optional type required");
        return item ? to_optional(std::forward<T>(item)) : to_optional(get_or_invoke(std::forward<U>(other)));
    }
};

struct conjunction_fn
{
    template<class T, class U>
    constexpr auto operator()(T&& item, U&& other) const
    {
        static_assert(is_optional<std::remove_reference_t<T>>{}, "opt::conjunction: optional type required");
        return !item ? to_optional(std::forward<T>(item)) : to_optional(get_or_invoke(std::forward<U>(other)));
    }
};
}  //namespace detail

namespace opt
{
static constexpr inline auto none = std::nullopt;
static constexpr inline auto some = detail::some_fn{};
static constexpr inline auto has_value = pipeable{ detail::has_value_fn{} };

static constexpr inline auto map = pipeable{ detail::map_fn{} };
static constexpr inline auto transform = map;

static constexpr inline auto flat_map = pipeable{ detail::flat_map_fn{} };
static constexpr inline auto and_then = flat_map;

static constexpr inline auto take_if = pipeable{ detail::filter_fn<true>{} };
static constexpr inline auto drop_if = pipeable{ detail::filter_fn<false>{} };
static constexpr inline auto filter = take_if;

static constexpr inline auto value = pipeable{ detail::value_fn{} };
static constexpr inline auto value_or = pipeable{ detail::value_or_fn{} };
static constexpr inline auto value_or_throw = pipeable{ detail::value_or_throw_fn{} };

static constexpr inline auto disjunction = pipeable{ detail::disjunction_fn{} };
static constexpr inline auto conjunction = pipeable{ detail::conjunction_fn{} };

static constexpr inline auto or_ = disjunction;
static constexpr inline auto and_ = conjunction;
}  // namespace opt

}  // namespace millrind
