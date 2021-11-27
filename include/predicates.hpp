#pragma once

#include <algorithm>
#include <iosfwd>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace predicates
{
namespace tags
{
struct negate
{
};

struct generic
{
};

struct all
{
};

struct any
{
};

}  // namespace tags

template<char... Ch>
struct t_str : std::integer_sequence<char, Ch...>
{
    static std::string_view to_string()
    {
        static const char result[] = { Ch..., '\0' };
        return result;
    }

    friend std::ostream& operator<<(std::ostream& os, const t_str& item)
    {
        return os << to_string();
    }
};

template<class Pred, class T>
constexpr bool match(const Pred& pred, const T& item)
{
    if constexpr (std::is_invocable_v<Pred, T>)
        return std::invoke(pred, item);
    else
        return item == pred;
}

template<class Tag, class... Args>
struct predicate_wrapper;

template<class Pred>
struct predicate_wrapper<tags::negate, Pred>
{
    Pred inner;

    template<class T>
    constexpr bool operator()(const T& item) const
    {
        return !match(inner, item);
    }

    friend std::ostream& operator<<(std::ostream& os, const predicate_wrapper& item)
    {
        return os << "!(" << item.inner << ")";
    }
};

template<class Pred, class Format, class... Args>
struct predicate_wrapper<tags::generic, Pred, Format, Args...>
{
    Pred inner;
    Format format;
    std::tuple<Args...> args;

    using index_seq = std::make_index_sequence<sizeof...(Args)>;

    template<class T>
    constexpr bool operator()(const T& item) const
    {
        return call(item, index_seq{});
    }

    friend std::ostream& operator<<(std::ostream& os, const predicate_wrapper& item)
    {
        item.output(os, index_seq{});
        return os;
    }

private:
    template<class T, size_t... I>
    constexpr bool call(const T& item, std::index_sequence<I...>) const
    {
        return std::invoke(inner, item, std::get<I>(args)...);
    }

    template<size_t... I>
    void output(std::ostream& os, std::index_sequence<I...>) const
    {
        format(os, std::get<I>(args)...);
    }
};

struct format_tuple_fn
{
    template<class Tuple, size_t... I>
    static void format(std::ostream& os, const Tuple& tuple, std::index_sequence<I...>)
    {
        (..., (os << (I == 0 ? "" : ", ") << std::get<I>(tuple)));
    }

    template<class Tuple>
    struct wrapper
    {
        const Tuple& tuple;

        friend std::ostream& operator<<(std::ostream& os, const wrapper& item)
        {
            format(os, item.tuple, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
            return os;
        }
    };

    template<class Tuple>
    constexpr auto operator()(const Tuple& tuple) const
    {
        return wrapper<Tuple>{ tuple };
    }
};

static constexpr inline auto format_tuple = format_tuple_fn{};

template<class... Preds>
struct predicate_wrapper<tags::all, Preds...>
{
    std::tuple<Preds...> preds;
    using index_seq = std::make_index_sequence<sizeof...(Preds)>;

    template<class T>
    constexpr bool operator()(const T& item) const
    {
        return call(item, index_seq{});
    }

    friend std::ostream& operator<<(std::ostream& os, const predicate_wrapper& item)
    {
        return os << "all(" << format_tuple(item.preds) << ")";
    }

private:
    template<class T, size_t... I>
    constexpr bool call(const T& item, std::index_sequence<I...>) const
    {
        return (match(std::get<I>(preds), item) && ...);
    }
};

template<class... Preds>
struct predicate_wrapper<tags::any, Preds...>
{
    std::tuple<Preds...> preds;
    using index_seq = std::make_index_sequence<sizeof...(Preds)>;

    template<class T>
    constexpr bool operator()(const T& item) const
    {
        return call(item, index_seq{});
    }

    friend std::ostream& operator<<(std::ostream& os, const predicate_wrapper& item)
    {
        return os << "any(" << format_tuple(item.preds) << ")";
    }

private:
    template<class T, size_t... I>
    constexpr bool call(const T& item, std::index_sequence<I...>) const
    {
        return (match(std::get<I>(preds), item) || ...);
    }
};

struct negate_fn
{
    template<class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return predicate_wrapper<tags::negate, Pred>{ std::move(pred) };
    }

    template<class Pred>
    constexpr auto operator()(predicate_wrapper<tags::negate, Pred> pred) const
    {
        return pred.inner;
    }
};

template<class... Args>
constexpr auto to_tuple(Args... args)
{
    return std::tuple<Args...>{ std::move(args)... };
}

struct build_fn
{
    template<class Pred, class Format>
    struct proxy
    {
        Pred pred;
        Format format;

        template<class... Args>
        constexpr auto operator()(Args... args) const
        {
            return predicate_wrapper<tags::generic, Pred, Format, Args...>{
                pred,
                format,
                to_tuple(std::move(args)...)
            };
        }
    };

    template<class Pred, class Format>
    constexpr auto operator()(Pred pred, Format format) const
    {
        return proxy<Pred, Format>{ std::move(pred), std::move(format) };
    }
};

template<class Tag>
struct complex_fn
{
    template<class... Preds>
    constexpr auto operator()(Preds... preds) const
    {
        return predicate_wrapper<Tag, Preds...>{ to_tuple(std::move(preds)...) };
    }

    template<class... L, class... R>
    constexpr auto operator()(predicate_wrapper<Tag, L...> lhs, predicate_wrapper<Tag, R...> rhs) const
    {
        return predicate_wrapper<Tag, L..., R...>{ std::tuple_cat(std::move(lhs.preds), std::move(rhs.preds)) };
    }

    template<class... L, class... R>
    constexpr auto operator()(predicate_wrapper<Tag, L...> lhs, R... rhs) const
    {
        return (*this)(std::move(lhs), (*this)(std::move(rhs...)));
    }
};

static constexpr inline auto all = complex_fn<tags::all>{};
static constexpr inline auto any = complex_fn<tags::any>{};

static constexpr inline auto negate = negate_fn{};
static constexpr inline auto build = build_fn{};

template<class... Args>
constexpr auto operator!(predicate_wrapper<Args...> pred)
{
    return negate(std::move(pred));
}

template<class... L, class... R>
constexpr auto operator&&(predicate_wrapper<L...> lhs, predicate_wrapper<R...> rhs)
{
    return all(std::move(lhs), std::move(rhs));
}

template<class... L, class... R>
constexpr auto operator||(predicate_wrapper<L...> lhs, predicate_wrapper<R...> rhs)
{
    return any(std::move(lhs), std::move(rhs));
}

struct each_fn
{
    template<class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return build(
            [](const auto& item, const auto& p) { return std::all_of(std::begin(item), std::end(item), [&](const auto& el) { return match(p, el); }); },
            [](std::ostream& os, const auto& p) { os << "each(" << p << ")"; })(std::move(pred));
    }
};

struct contains_fn
{
    template<class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return build(
            [](const auto& item, const auto& p) { return std::any_of(std::begin(item), std::end(item), [&](const auto& el) { return match(p, el); }); },
            [](std::ostream& os, const auto& p) { os << "contains(" << p << ")"; })(std::move(pred));
    }

    template<class Pred, class Times>
    constexpr auto operator()(Pred pred, Times times) const
    {
        return build(
            [](const auto& item, const auto& p, const auto& t) { return match(t, std::count_if(std::begin(item), std::end(item), [&](const auto& el) { return match(p, el); })); },
            [](std::ostream& os, const auto& p, const auto& t) { os << "contains(" << p << ", " << t << ")"; })(std::move(pred), std::move(times));
    }
};

struct is_empty_fn
{
    constexpr auto operator()() const
    {
        return build(
            [](const auto& item) { return std::begin(item) == std::end(item); },
            [](std::ostream& os) { os << "is_empty()"; })();
    }
};

struct size_is_fn
{
    template<class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return build(
            [](const auto& item, const auto& p) { return match(p, std::distance(std::begin(item), std::end(item))); },
            [](std::ostream& os, const auto& p) { os << "size_is(" << p << ")"; })(std::move(pred));
    }
};

template<class T>
struct variant_with_fn
{
    constexpr auto operator()() const
    {
        return build(
            [](const auto& item) { return std::holds_alternative<T>(item); },
            [](std::ostream& os) { os << "variant_with"; })();
    }

    template<class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return build(
            [](const auto& item, const auto& p) { return std::holds_alternative<T>(item) && match(p, std::get<T>(item)); },
            [](std::ostream& os, const auto& p) { os << "variant_with(" << p << ")"; })(std::move(pred));
    }
};

struct result_of_fn
{
    template<class Func, class Pred>
    constexpr auto operator()(Func func, Pred pred) const
    {
        return build(
            [](const auto& item, const auto& f, const auto& p) { return match(p, std::invoke(f, item)); },
            [](std::ostream& os, const auto& f, const auto& p) { os << "result_of(" << f << ", " << p << ")"; })(std::move(func), std::move(pred));
    }
};

struct alias_adapter
{
    std::string name;

    template<class Pred>
    friend auto operator>>(Pred pred, const alias_adapter& adapter)
    {
        return build(
            std::move(pred),
            [=](std::ostream& os) { os << adapter.name; })();
    }
};

struct alias_fn
{
    auto operator()(std::string name) const
    {
        return alias_adapter{ std::move(name) };
    }

    template<class Pred>
    constexpr auto operator()(Pred pred, std::string name) const
    {
        return build(
            std::move(pred),
            [=](std::ostream& os) { os << name; })();
    }
};

struct unary_formatter
{
    std::string_view name;

    template<class T>
    void operator()(std::ostream& os, const T& arg) const
    {
        os << name << "(" << arg << ")";
    }
};

static constexpr inline auto eq = build(std::equal_to<>{}, unary_formatter{ "eq" });
static constexpr inline auto ne = build(std::not_equal_to<>{}, unary_formatter{ "ne" });

static constexpr inline auto gt = build(std::greater<>{}, unary_formatter{ "gt" });
static constexpr inline auto ge = build(std::greater_equal<>{}, unary_formatter{ "ge" });

static constexpr inline auto lt = build(std::less<>{}, unary_formatter{ "lt" });
static constexpr inline auto le = build(std::less_equal<>{}, unary_formatter{ "le" });

static constexpr inline auto each = each_fn{};
static constexpr inline auto contains = contains_fn{};
static constexpr inline auto is_empty = is_empty_fn{};
static constexpr inline auto size_is = size_is_fn{};

static constexpr inline auto result_of = result_of_fn{};
static constexpr inline auto field = result_of_fn{};

template<class T>
static constexpr inline auto variant_with = variant_with_fn<T>{};

static constexpr inline auto alias = alias_fn{};

}  // namespace predicates
