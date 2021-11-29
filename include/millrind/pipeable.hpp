#pragma once

#include <functional>

#include "wrappers.hpp"

namespace millrind
{
namespace detail
{
struct invoke_unary_fn
{
    template<class Func, class T>
    constexpr decltype(auto) operator()(Func&& func, T&& arg) const
    {
        return (*this)(
            std::forward<Func>(func),
            std::forward<T>(arg),
            std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{});
    }

private:
    template<class Func, class T, size_t... I>
    constexpr decltype(auto) operator()(Func&& func, T&& arg, std::index_sequence<I...>) const
    {
        return std::invoke(std::forward<Func>(func), unwrap(std::get<I>(std::forward<T>(arg)))...);
    }
};

static constexpr inline auto invoke_unary = invoke_unary_fn{};

struct call_fn
{
    template<class Func, class... Args>
    constexpr decltype(auto) operator()(Func&& func, Args&&... args) const
    {
        return std::invoke(std::forward<Func>(func), unwrap(std::forward<Args>(args))...);
    }

    template<class Func, class Arg>
    constexpr decltype(auto) operator()(Func&& func, Arg&& arg) const
    {
        if constexpr (std::is_invocable_v<decltype(std::forward<Func>(func)), decltype(unwrap(std::forward<Arg>(arg)))>)
            return std::invoke(std::forward<Func>(func), unwrap(std::forward<Arg>(arg)));
        else
            return invoke_unary(std::forward<Func>(func), unwrap(std::forward<Arg>(arg)));
    }
};

static constexpr inline auto call = call_fn{};

template<class Func, class... Args>
using is_callable = decltype(call(std::declval<Func>(), std::declval<Args>()...));

template<class F, class G>
struct function_composition
{
    F f;
    G g;

    template<class... Args>
    constexpr decltype(auto) operator()(Args&&... args) const
    {
        return call(g, call(f, std::forward<Args>(args)...));
    }
};

template<class F, class G>
function_composition(F, G) -> function_composition<F, G>;

template<class Func>
struct pipeable_adaptor
{
    Func func;

    template<class... Args>
    constexpr decltype(auto) operator()(Args&&... args) const
    {
        return call(func, std::forward<Args>(args)...);
    }
};

template<class Func>
pipeable_adaptor(Func) -> pipeable_adaptor<Func>;

template<class T, class Func>
constexpr decltype(auto) operator|(T&& item, const pipeable_adaptor<Func>& pipeable_adaptor)
{
    return pipeable_adaptor(std::forward<T>(item));
}

template<class L, class R>
constexpr auto operator|(pipeable_adaptor<L> lhs, pipeable_adaptor<R> rhs)
{
    return pipeable_adaptor{ function_composition{ std::move(lhs.func), std::move(rhs.func) } };
}

template<class Func>
struct pipeable
{
    Func func;

    template<class... Args>
    struct impl
    {
        Func func;
        std::tuple<Args...> args;

        template<class T>
        constexpr decltype(auto) operator()(T&& item) const
        {
            return do_call(std::forward<T>(item), std::make_index_sequence<sizeof...(Args)>{});
        }

        template<class T, size_t... I>
        constexpr decltype(auto) do_call(T&& item, std::index_sequence<I...>) const
        {
            return call(func, std::forward<T>(item), std::get<I>(args)...);
        }
    };

    template<class... Args>
    constexpr auto operator()(Args&&... args) const
    {
        return pipeable_adaptor{ impl<std::decay_t<Args>...>{ func, std::tuple{ std::forward<Args>(args)... } } };
    }
};

template<class Func>
pipeable(Func) -> pipeable<Func>;

struct pipe_fn
{
    template<class Head, class... Tail>
    constexpr auto operator()(Head&& head, Tail&&... tail) const
    {
        if constexpr (sizeof...(tail) > 0)
            return pipeable_adaptor{ std::forward<Head>(head) } | (*this)(std::forward<Tail>(tail)...);
        else
            return pipeable_adaptor{ std::forward<Head>(head) };
    }
};

struct tee_fn
{
    template<class T, class Func>
    constexpr T&& operator()(T&& item, Func&& func) const
    {
        call(std::forward<Func>(func), item);
        return std::forward<T>(item);
    }
};

}  // namespace detail

struct identity
{
    template<class T>
    constexpr T&& operator()(T&& item) const noexcept
    {
        return std::forward<T>(item);
    }
};

using detail::call;
using detail::pipeable;
using detail::pipeable_adaptor;

static constexpr inline auto pipe = detail::pipe_fn{};
static constexpr inline auto fn = pipe;

static constexpr inline auto tee = pipeable{ detail::tee_fn{} };

}  // namespace millrind
