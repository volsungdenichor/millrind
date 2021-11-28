#pragma once

#include "algorithm.hpp"
#include "iterator_range.hpp"
#include "iterators/any_iterator.hpp"
#include "iterators/cache_lastest_iterator.hpp"
#include "iterators/chain_iterator.hpp"
#include "iterators/enumerating_iterator.hpp"
#include "iterators/filter_iterator.hpp"
#include "iterators/filter_map_iterator.hpp"
#include "iterators/flat_map_iterator.hpp"
#include "iterators/generating_iterator.hpp"
#include "iterators/iterate_iterator.hpp"
#include "iterators/map_iterator.hpp"
#include "iterators/numeric_iterator.hpp"
#include "iterators/owning_iterator.hpp"
#include "iterators/repeat_iterator.hpp"
#include "iterators/stride_iterator.hpp"
#include "iterators/zip_transform_iterator.hpp"

namespace millrind
{
namespace seq
{
namespace detail
{
struct reverse_fn
{
    template<class Range>
    auto operator()(Range&& range) const
    {
        return create(std::begin(range), std::end(range));
    }

    template<class Iter>
    auto create(Iter b, Iter e) const
    {
        using result_type = std::reverse_iterator<Iter>;
        return make_range(result_type{ e }, result_type{ b });
    }

    template<class Iter>
    auto create(std::reverse_iterator<Iter> b, std::reverse_iterator<Iter> e) const
    {
        using result_type = Iter;
        return make_range(result_type{ e.base() }, result_type{ b.base() });
    }
};

template<class Iter, class A, class... Args>
auto _last(iterator_range<Iter> range, A p, Args&&... args) -> iterator_range<Iter>
{
    static const auto reverse = pipeable{ reverse_fn{} }();
    const auto pipeable_adaptor = pipeable{ p }(std::forward<Args>(args)...);
    return make_range(range) | reverse | pipeable_adaptor | reverse;
}

template<class Iter, class A, class... Args>
auto _both(iterator_range<Iter> range, A p, Args&&... args) -> iterator_range<Iter>
{
    static const auto reverse = pipeable{ reverse_fn{} }();
    const auto pipeable_adaptor = pipeable{ p }(std::forward<Args>(args)...);
    return make_range(range) | pipeable_adaptor | reverse | pipeable_adaptor | reverse;
}

enum class direction
{
    left,
    right,
    both
};

struct generate_fn
{
    template<class Func>
    auto operator()(Func func) const
    {
        using result_type = generating_iterator<Func>;
        return make_range(result_type{ func }, result_type{});
    }
};

template<direction Dir>
struct take_fn
{
    template<class Range>
    constexpr auto operator()(Range&& range, std::ptrdiff_t count) const
    {
        if constexpr (Dir == direction::left)
        {
            return create(std::begin(range), std::end(range), count);
        }
        else if constexpr (Dir == direction::right)
        {
            return _last(make_range(range), take_fn<direction::left>{}, count);
        }
    }

    template<class Iter>
    constexpr auto create(Iter b, Iter e, std::ptrdiff_t count) const
    {
        if constexpr (is_detected_v<random_access_iterator, Iter>)
            return make_range(b, advance(b, count, e));
        else
        {
            return generate_fn{}([=]() mutable -> std::optional<wrapped<iter_reference_t<Iter>>> {
                if (count-- > 0 && b != e)
                    return *b++;
                else
                    return std::nullopt;
            });
        }
    }
};

template<direction Dir>
struct drop_fn
{
    template<class Range>
    constexpr auto operator()(Range&& range, std::ptrdiff_t count) const
    {
        if constexpr (Dir == direction::left)
        {
            return create(std::begin(range), std::end(range), count);
        }
        else if constexpr (Dir == direction::right)
        {
            return _last(make_range(range), drop_fn<direction::left>{}, count);
        }
        else if constexpr (Dir == direction::both)
        {
            return _both(make_range(range), drop_fn<direction::left>{}, count);
        }
    }

    template<class Iter>
    constexpr auto create(Iter b, Iter e, std::ptrdiff_t count) const
    {
        return make_range(advance(b, count, e), e);
    }
};

template<direction Dir, bool Expected>
struct take_while_fn
{
    template<class Range, class Pred, class Proj = identity>
    constexpr auto operator()(Range&& range, Pred pred, Proj proj = {}) const
    {
        if constexpr (Dir == direction::left)
        {
            auto b = std::begin(range);
            auto e = std::end(range);
            return make_range(b, advance_while<Expected>(b, fn(ref{ proj }, ref{ pred }), e));
        }
        else if constexpr (Dir == direction::right)
        {
            return _last(make_range(range), take_while_fn<direction::left, Expected>{}, pred, proj);
        }
    }
};

template<direction Dir, bool Expected>
struct drop_while_fn
{
    template<class Range, class Pred, class Proj = identity>
    constexpr auto operator()(Range&& range, Pred pred, Proj proj = {}) const
    {
        if constexpr (Dir == direction::left)
        {
            auto b = std::begin(range);
            auto e = std::end(range);
            return make_range(advance_while<Expected>(b, fn(ref{ proj }, ref{ pred }), e), e);
        }
        else if constexpr (Dir == direction::right)
        {
            return _last(make_range(range), drop_while_fn<direction::left, Expected>{}, pred, proj);
        }
        else if constexpr (Dir == direction::both)
        {
            return _both(make_range(range), drop_while_fn<direction::left, Expected>{}, pred, proj);
        }
    }
};

struct stride_fn
{
    template<class Range>
    auto operator()(Range&& range, std::ptrdiff_t step) const
    {
        return create(std::begin(range), std::end(range), step);
    }

    template<class Iter>
    auto create(Iter b, Iter e, std::ptrdiff_t step) const
    {
        using result_type = stride_iterator<Iter>;
        return make_range(result_type{ b, step, e }, result_type{ e, step, e });
    }
};

struct iterate_fn
{
    template<class Range>
    auto operator()(Range&& range) const
    {
        return create(std::begin(range), std::end(range));
    }

    template<class Iter>
    auto create(Iter b, Iter e) const
    {
        using result_type = iterate_iterator<Iter>;

        return make_range(result_type{ std::move(b) }, result_type{ std::move(e) });
    }
};

struct map_fn
{
    template<class Range, class Func, class Proj = identity>
    auto operator()(Range&& range, Func func, Proj proj = {}) const
    {
        return create(std::begin(range), std::end(range), fn(std::move(proj), std::move(func)));
    }

    template<class Iter, class Func>
    auto create(Iter b, Iter e, Func func) const
    {
        using result_type = map_iterator<Func, Iter>;

        return make_range(result_type{ func, std::move(b) }, result_type{ func, std::move(e) });
    }
};

template<bool Expected>
struct filter_fn
{
    template<class Range, class Pred, class Proj = identity>
    auto operator()(Range&& range, Pred pred, Proj proj = {}) const
    {
        if constexpr (Expected)
            return create(std::begin(range), std::end(range), fn(std::move(proj), std::move(pred)));
        else
            return create(std::begin(range), std::end(range), fn(std::move(proj), std::not_fn(pred)));
    }

    template<class Iter, class Pred>
    auto create(Iter b, Iter e, Pred pred) const
    {
        using result_type = filter_iterator<Pred, Iter>;

        return make_range(result_type{ pred, b, e }, result_type{ pred, e, e });
    }
};

struct flat_map_fn
{
    template<class Range, class Func, class Proj = identity>
    auto operator()(Range&& range, Func pred, Proj proj = {}) const
    {
        return create(std::begin(range), std::end(range), fn(std::move(proj), std::move(pred)));
    }

    template<class Iter, class Func>
    auto create(Iter b, Iter e, Func func) const
    {
        using result_type = flat_map_iterator<Func, Iter>;

        return make_range(result_type{ func, b, e }, result_type{ func, e, e });
    }
};

struct flatten_fn
{
    template<class Range>
    auto operator()(Range&& range) const
    {
        return flat_map_fn{}(make_range(range), identity{});
    }
};

struct filter_map_fn
{
    template<class Range, class Func, class Proj = identity>
    auto operator()(Range&& range, Func pred, Proj proj = {}) const
    {
        return create(std::begin(range), std::end(range), fn(std::move(proj), std::move(pred)));
    }

    template<class Iter, class Func>
    auto create(Iter b, Iter e, Func func) const
    {
        using result_type = filter_map_iterator<Func, Iter>;

        return make_range(result_type{ func, b, e }, result_type{ func, e, e });
    }
};

struct enumerate_fn
{
    template<class Range>
    auto operator()(Range&& range, std::ptrdiff_t start = 0) const
    {
        return create(std::begin(range), std::end(range), start);
    }

    template<class Iter>
    auto create(Iter b, Iter e, std::ptrdiff_t start) const
    {
        using result_type = enumerating_iterator<Iter>;

        if constexpr (is_detected_v<random_access_iterator, Iter>)
            return make_range(result_type{ b, start }, result_type{ e, start + std::distance(b, e) });
        else
            return make_range(result_type{ b, start }, result_type{ e });
    }
};

struct concat_fn
{
    template<class Range>
    auto operator()(Range&& range) const
    {
        return make_range(std::begin(range), std::end(range));
    }

    template<class Range1, class Range2>
    auto operator()(Range1&& range1, Range2&& range2) const
    {
        return create(std::begin(range1), std::end(range1), std::begin(range2), std::end(range2));
    }

    template<class Range1, class Range2, class... Tail>
    auto operator()(Range1&& range1, Range2&& range2, Tail&&... tail) const
    {
        return (*this)((*this)(make_range(range1), make_range(range2)), std::forward<Tail>(tail)...);
    }

    template<class Iter1, class Iter2>
    auto create(Iter1 b1, Iter1 e1, Iter2 b2, Iter2 e2) const
    {
        using result_type = chain_iterator<Iter1, Iter2>;

        return make_range(result_type{ b1, b2, e1, e2 }, result_type{ e1, e2, e1, e2 });
    }
};

struct zip_transform_fn
{
    template<class Func, class... Ranges>
    auto operator()(const Func& func, Ranges&&... ranges) const
    {
        return make_range(
            zip_transform_iterator{ func, std::begin(ranges)... },
            zip_transform_iterator{ func, std::end(ranges)... });
    }
};

struct zip_fn
{
    struct to_tuple_fn
    {
        template<class... Args>
        constexpr auto operator()(Args&&... args) const
        {
            return std::tuple<Args...>{ std::forward<Args>(args)... };
        }
    };

    template<class... Ranges>
    auto operator()(Ranges&&... ranges) const
    {
        return zip_transform_fn{}(to_tuple_fn{}, make_range(ranges)...);
    }
};

struct adjacent_fn
{
    template<class Range>
    auto operator()(Range&& range) const
    {
        return zip_fn{}(
            drop_fn<direction::right>{}(make_range(range), 1),
            drop_fn<direction::left>{}(make_range(range), 1));
    }
};

struct adjacent_transform_fn
{
    template<class Range, class Func, class Proj = identity>
    auto operator()(Range&& range, Func func, Proj proj = {}) const
    {
        return zip_transform_fn{}(
            ::millrind::detail::invoke_binary{ func, proj },
            drop_fn<direction::right>{}(make_range(range), 1),
            drop_fn<direction::left>{}(make_range(range), 1));
    }
};

struct iota_fn
{
    template<class T>
    auto operator()(T lo, T up) const
    {
        using result_type = numeric_iterator<T>;
        return make_range(result_type{ lo }, result_type{ up });
    }

    template<class T>
    auto operator()(T up) const
    {
        return (*this)(T{}, up);
    }
};

struct owned_fn
{
    template<class Container>
    auto operator()(Container container) const
    {
        using result_type = owning_iterator<iterator_t<Container>, Container>;
        auto ptr = std::make_shared<Container>(std::move(container));
        return make_range(result_type{ std::begin(*ptr), ptr }, result_type{ std::end(*ptr), ptr });
    }

    template<class T>
    auto operator()(std::initializer_list<T> init) const
    {
        return (*this)(std::vector<T>{ init });
    }
};

struct repeat_fn
{
    template<class T>
    auto operator()(T value, std::ptrdiff_t count) const
    {
        using result_type = repeat_iterator<T>;
        return make_range(result_type{ value, 0 }, result_type{ value, count });
    }
};

struct copy_fn
{
    template<class Range, class Output>
    auto operator()(Range&& range, Output output) const
    {
        return copy(make_range(range), output);
    }
};

struct for_each_fn
{
    template<class Range, class Func, class Proj = identity>
    auto operator()(Range&& range, Func func, Proj proj = {}) const
    {
        return for_each(make_range(range), ref{ func }, ref{ proj });
    }
};

struct cache_latest_fn
{
    template<class Range>
    auto operator()(Range&& range) const
    {
        return create(std::begin(range), std::end(range));
    }

    template<class Iter>
    auto create(Iter b, Iter e) const
    {
        using result_type = cache_latest_iterator<Iter>;
        return make_range(result_type{ b }, result_type{ e });
    }
};

struct front_fn
{
    template<class Range>
    decltype(auto) operator()(Range&& range) const
    {
        auto b = std::begin(range);
        auto e = std::end(range);
        if (b == e)
            throw std::runtime_error{ "seq::front: empty range" };
        return unwrap(*b);
    }
};

}  // namespace detail

static constexpr inline auto iterate = pipeable{ detail::iterate_fn{} };
static constexpr inline auto map = pipeable{ detail::map_fn{} };
static constexpr inline auto transform = map;
static constexpr inline auto take_if = pipeable{ detail::filter_fn<true>{} };
static constexpr inline auto drop_if = pipeable{ detail::filter_fn<false>{} };
static constexpr inline auto filter = take_if;
static constexpr inline auto flat_map = pipeable{ detail::flat_map_fn{} };
static constexpr inline auto transform_join = flat_map;
static constexpr inline auto flatten = pipeable{ detail::flatten_fn{} };
static constexpr inline auto join = flatten;
static constexpr inline auto filter_map = pipeable{ detail::filter_map_fn{} };
static constexpr inline auto transform_maybe = filter_map;
static constexpr inline auto enumerate = pipeable{ detail::enumerate_fn{} };
static constexpr inline auto reverse = pipeable{ detail::reverse_fn{} };

static constexpr inline auto stride = pipeable{ detail::stride_fn{} };

static constexpr inline auto take = pipeable{ detail::take_fn<detail::direction::left>{} };
static constexpr inline auto drop = pipeable{ detail::drop_fn<detail::direction::left>{} };
static constexpr inline auto take_while = pipeable{ detail::take_while_fn<detail::direction::left, true>{} };
static constexpr inline auto drop_while = pipeable{ detail::drop_while_fn<detail::direction::left, true>{} };
static constexpr inline auto take_until = pipeable{ detail::take_while_fn<detail::direction::left, false>{} };
static constexpr inline auto drop_until = pipeable{ detail::drop_while_fn<detail::direction::left, false>{} };

static constexpr inline auto take_last = pipeable{ detail::take_fn<detail::direction::right>{} };
static constexpr inline auto drop_last = pipeable{ detail::drop_fn<detail::direction::right>{} };
static constexpr inline auto take_last_while = pipeable{ detail::take_while_fn<detail::direction::right, true>{} };
static constexpr inline auto drop_last_while = pipeable{ detail::drop_while_fn<detail::direction::right, true>{} };
static constexpr inline auto take_last_until = pipeable{ detail::take_while_fn<detail::direction::right, false>{} };
static constexpr inline auto drop_last_until = pipeable{ detail::drop_while_fn<detail::direction::right, false>{} };

static constexpr inline auto trim = pipeable{ detail::drop_fn<detail::direction::both>{} };
static constexpr inline auto trim_while = pipeable{ detail::drop_while_fn<detail::direction::both, true>{} };
static constexpr inline auto trim_until = pipeable{ detail::drop_while_fn<detail::direction::both, false>{} };

static constexpr inline auto cache_latest = pipeable{ detail::cache_latest_fn{} };

static constexpr inline auto adjacent = pipeable{ detail::adjacent_fn{} };
static constexpr inline auto adjacent_transform = pipeable{ detail::adjacent_transform_fn{} };

static constexpr inline auto copy = pipeable{ detail::copy_fn{} };
static constexpr inline auto for_each = pipeable{ detail::for_each_fn{} };

static constexpr inline auto front = pipeable{ detail::front_fn{} };

static constexpr inline auto zip = detail::zip_fn{};
static constexpr inline auto zip_transform = detail::zip_transform_fn{};

static constexpr inline auto concat = detail::concat_fn{};
static constexpr inline auto iota = detail::iota_fn{};
static constexpr inline auto owned = detail::owned_fn{};
static constexpr inline auto repeat = detail::repeat_fn{};
static constexpr inline auto generate = detail::generate_fn{};

}  // namespace seq

template<class T>
using iterable = iterator_range<any_iterator<T>>;

}  // namespace millrind