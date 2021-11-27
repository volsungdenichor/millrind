#pragma once

#include "algorithm.hpp"

namespace millrind
{
namespace detail
{
template<class Iter, class Output, class Func, class UnaryPred>
Output transform_if(Iter b, Iter e, Output output, Func func, UnaryPred pred)
{
    for (; b != e; ++b)
    {
        if (pred(*b))
        {
            yield(output, func(*b));
        }
    }
    return output;
}

template<class Iter, class Output, class UnaryPred>
Output copy_while(Iter b, Iter e, Output output, UnaryPred pred)
{
    for (; b != e && pred(*b); ++b)
    {
        yield(output, *b);
    }
    return output;
}

template<class Iter1, class Iter2, class Func>
Iter1 overwrite(Iter1 src_b, Iter1 src_e, Iter2 dst_b, Iter2 dst_e, Func func)
{
    for (; src_b != src_e && dst_b != dst_e; ++src_b, ++dst_b)
    {
        *dst_b = std::invoke(func, *src_b);
    }
    return src_b;
}

}  // namespace detail
template<class Range, class Output, class Func, class UnaryPred, class Proj = identity>
auto transform_if(Range&& range, Output output, Func func, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("transform_if", range, input_range);

    return detail::transform_if(std::begin(range), std::end(range), output, ref(func), fn(ref(proj), ref(pred)));
}

template<class Range, class Output, class UnaryPred, class Proj = identity>
auto copy_while(Range&& range, Output output, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("copy_while", range, input_range);

    return detail::copy_while(std::begin(range), std::end(range), output, fn(ref(proj), ref(pred)));
}

template<class Range, class Output, class UnaryPred, class Proj = identity>
auto copy_until(Range&& range, Output output, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("copy_until", range, input_range);

    return detail::copy_while(std::begin(range), std::end(range), output, std::not_fn(fn(ref(proj), ref(pred))));
}

template<class Range, class Dest>
auto overwrite(Range&& range, Dest&& dest)
{
    MILLRIND_CHECK_CONSTRAINT("overwrite", range, input_range);

    return detail::overwrite(std::begin(range), std::end(range), std::begin(dest), std::end(dest), identity{});
}

template<class Range, class Dest, class UnaryFunc, class Proj = identity>
auto transform_overwrite(Range&& range, Dest&& dest, UnaryFunc func, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("copy_transform_overwrite", range, input_range);

    return detail::overwrite(std::begin(range), std::end(range), std::begin(dest), std::end(dest), fn(ref(proj), ref(func)));
}

template<class Range, class BinaryPred = std::equal_to<>, class Proj = identity>
bool all_equal(Range&& range, BinaryPred&& pred = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("all_equal", range, forward_range);

    auto b = std::begin(range);
    auto e = std::end(range);
    return b == e || std::all_of(std::next(b), e, [&](auto&& item) { return call(pred, call(proj, std::forward<decltype(item)>(item)), call(proj, *b)); });
}

template<class Range1, class Range2, class BinaryPred = std::equal_to<>, class Proj1 = identity, class Proj2 = identity>
bool starts_with(Range1&& range1, Range2&& range2, BinaryPred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("starts_with", range1, forward_range);
    MILLRIND_CHECK_CONSTRAINT("starts_with", range2, forward_range);

    auto r1 = make_range(range1);
    auto r2 = make_range(range2);
    return search<return_found>(r1, r2, ref(pred), ref(proj1), ref(proj2)) == std::begin(r1);
}

}  // namespace millrind