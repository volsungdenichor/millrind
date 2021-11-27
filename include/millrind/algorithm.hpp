#pragma once

#include <algorithm>
#include <functional>
#include <numeric>

#include "pipeable.hpp"
#include "return_policy.hpp"

#define MILLRIND_CHECK_CONSTRAINT(func, var, constraint) \
    static_assert(is_detected_v<constraint, decltype(var)>, func ": '" #var "' - " #constraint " required")

namespace millrind
{
namespace detail
{
template<class Func, class Proj1, class Proj2>
struct invoke_binary
{
    Func func;
    Proj1 proj1;
    Proj2 proj2;

    template<class T, class U>
    constexpr decltype(auto) operator()(T&& lhs, U&& rhs) const
    {
        return call(func, call(proj1, std::forward<T>(lhs)), call(proj2, std::forward<U>(rhs)));
    }
};

template<class Func, class Proj>
struct invoke_binary<Func, Proj, void>
{
    Func func;
    Proj proj;

    template<class T, class U>
    constexpr decltype(auto) operator()(T&& lhs, U&& rhs) const
    {
        return call(func, call(proj, std::forward<T>(lhs)), call(proj, std::forward<U>(rhs)));
    }
};

template<class Func, class Proj>
invoke_binary(Func, Proj) -> invoke_binary<Func, Proj, void>;

template<class Func, class Proj1, class Proj2>
invoke_binary(Func, Proj1, Proj2) -> invoke_binary<Func, Proj1, Proj2>;

template<class Output, class T>
void yield(Output& output, T&& item)
{
    *output = std::forward<T>(item);
    ++output;
}

template<class Policy, class Iter, class Func>
decltype(auto) invoke_algorithm(Iter b, Iter e, Func&& func)
{
    static const auto policy = Policy{};
    auto it = std::invoke(std::forward<Func>(func), b, e);
    return policy(it, b, e);
}

template<class Iter, class OutputIter, class BinaryFunc, class Proj>
auto adjacent_difference(Iter b, Iter e, OutputIter output, BinaryFunc func, Proj proj)
{
    if (b == e)
        return output;

    auto prev = call(proj, *b);
    yield(output, prev);
    while (++b != e)
    {
        auto cur = call(proj, *b);
        yield(output, call(func, cur, std::move(prev)));
        prev = std::move(cur);
    }
    return output;
}

template<class Iter, class Op>
Iter get_bound(Iter b, Iter e, Op op)
{
    Iter it;
    auto count = std::distance(b, e);
    auto step = count / 2;

    while (count > 0)
    {
        it = b;
        step = count / 2;
        std::advance(it, step);
        if (op(it))
        {
            b = ++it;
            count -= step + 1;
        }
        else
            count = step;
    }
    return b;
}

template<class Iter, class T, class Compare, class Proj>
Iter lower_bound(Iter b, Iter e, const T& value, Compare compare, Proj proj)
{
    return get_bound(b, e, [&](auto it) { return call(compare, call(proj, *it), value); });
}

template<class Iter, class T, class Compare, class Proj>
Iter upper_bound(Iter b, Iter e, const T& value, Compare compare, Proj proj)
{
    return get_bound(b, e, [&](auto it) { return !call(compare, value, call(proj, *it)); });
}

template<class Iter, class T, class Compare, class Proj>
auto equal_range(Iter b, Iter e, const T& value, Compare compare, Proj proj) -> iterator_range<Iter>
{
    return { lower_bound(b, e, value, ref{ compare }, ref{ proj }),
             upper_bound(b, e, value, ref{ compare }, ref{ proj }) };
}

template<class Iter, class Output, class BinaryFunc, class Proj>
Output partial_sum(Iter b, Iter e, Output output, BinaryFunc func, Proj proj)
{
    if (b == e)
        return output;

    auto sum = call(proj, *b);
    yield(output, sum);

    while (++b != e)
    {
        sum = call(func, std::move(sum), call(proj, *b));
        yield(output, sum);
    }
    return output;
}

}  // namespace detail
template<class Range, class T, class BinaryFunc = std::plus<>, class Proj = identity>
auto accumulate(Range&& range, T init, BinaryFunc func = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("accumulate", range, input_range);

    return std::accumulate(std::begin(range), std::end(range), init, [&](T total, auto&& item) {
        return call(func, std::move(total), call(proj, std::forward<decltype(item)>(item)));
    });
}

template<class Range, class OutputIter, class BinaryFunc = std::minus<>, class Proj = identity>
auto adjacent_difference(Range&& range, OutputIter output, BinaryFunc func = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("adjacent_difference", range, input_range);

    return detail::adjacent_difference(std::begin(range), std::end(range), output, ref{ func }, ref{ proj });
}

template<class Range, class UnaryPred, class Proj = identity>
auto all_of(Range&& range, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("all_of", range, input_range);

    return std::all_of(std::begin(range), std::end(range), fn(ref{ proj }, ref{ pred }));
}

template<class Range, class UnaryPred, class Proj = identity>
auto any_of(Range&& range, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("any_of", range, input_range);

    return std::any_of(std::begin(range), std::end(range), fn(ref{ proj }, ref{ pred }));
}

template<class Range, class OutputIter>
auto copy(Range&& range, OutputIter output)
{
    MILLRIND_CHECK_CONSTRAINT("copy", range, input_range);

    return std::copy(std::begin(range), std::end(range), output);
}

template<class Range, class OutputIter, class UnaryPred, class Proj = identity>
auto copy_if(Range&& range, OutputIter output, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("copy_if", range, input_range);

    return std::copy_if(std::begin(range), std::end(range), output, fn(ref{ proj }, ref{ pred }));
}

template<class Range, class Size, class OutputIter>
auto copy_n(Range&& range, Size size, OutputIter output)
{
    MILLRIND_CHECK_CONSTRAINT("copy_n", range, input_range);

    return std::copy_n(std::begin(range), size, output);
}

template<class Range, class T, class Proj = identity>
auto count(Range&& range, const T& value, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("count", range, input_range);

    return std::count_if(std::begin(range), std::end(range), fn(ref{ proj }, equal_to(ref{ value })));
}

template<class Range, class UnaryPred, class Proj = identity>
auto count_if(Range&& range, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("count_if", range, input_range);

    return std::count_if(std::begin(range), std::end(range), fn(ref{ proj }, ref{ pred }));
}

template<class Range1, class Range2, class BinaryPred = std::equal_to<>, class Proj1 = identity, class Proj2 = identity>
auto equal(Range1&& range1, Range2&& range2, BinaryPred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("equal", range1, input_range);
    MILLRIND_CHECK_CONSTRAINT("equal", range2, input_range);

    return std::equal(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        detail::invoke_binary{ ref{ pred }, ref{ proj1 }, ref{ proj2 } });
}

template<class Range, class T, class Compare = std::less<>, class Proj = identity>
auto equal_range(Range&& range, const T& value, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("equal_range", range, forward_range);

    return detail::equal_range(std::begin(range), std::end(range), value, ref{ compare }, ref{ proj });
}

template<class Range, class Output, class T, class BinaryFunc, class Proj = identity>
auto exclusive_scan(Range&& range, Output output, T init, BinaryFunc func, Proj proj = {})
{
    return std::transform_exclusive_scan(std::begin(range), std::end(range), output, init, ref{ func }, ref{ proj });
}

template<class Range, class T>
void fill(Range&& range, const T& value)
{
    MILLRIND_CHECK_CONSTRAINT("fill", range, forward_range);

    std::fill(std::begin(range), std::end(range), value);
}

template<class Policy = default_return_policy, class Range, class T, class Proj = identity>
decltype(auto) find(Range&& range, const T& value, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("find", range, input_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::find_if(b, e, fn(ref{ proj }, equal_to(ref{ value })));
    });
}

template<class Policy = default_return_policy, class Range, class UnaryPred, class Proj = identity>
decltype(auto) find_if(Range&& range, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("find_if", range, input_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::find_if(b, e, fn(ref{ proj }, ref{ pred }));
    });
}

template<class Policy = default_return_policy, class Range, class UnaryPred, class Proj = identity>
decltype(auto) find_if_not(Range&& range, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("find_if_not", range, input_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::find_if_not(b, e, fn(ref{ proj }, ref{ pred }));
    });
}

template<
    class Policy = default_return_policy,
    class Range1,
    class Range2,
    class BinaryPred = std::equal_to<>,
    class Proj1 = identity,
    class Proj2 = identity>
decltype(auto) find_end(Range1&& range1, Range2&& range2, BinaryPred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("find_end", range1, forward_range);
    MILLRIND_CHECK_CONSTRAINT("find_end", range2, forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range1), std::end(range1), [&](auto b, auto e) {
        return std::find_end(
            b,
            e,
            std::begin(range2),
            std::end(range2),
            detail::invoke_binary{ ref{ pred }, ref{ proj1 }, ref{ proj2 } });
    });
}

template<
    class Policy = default_return_policy,
    class Range1,
    class Range2,
    class BinaryPred = std::equal_to<>,
    class Proj1 = identity,
    class Proj2 = identity>
decltype(auto) find_first_of(Range1&& range1, Range2&& range2, BinaryPred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("find_first_of", range1, forward_range);
    MILLRIND_CHECK_CONSTRAINT("find_first_of", range2, forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range1), std::end(range1), [&](auto b, auto e) {
        return std::find_first_of(
            b,
            e,
            std::begin(range2),
            std::end(range2),
            detail::invoke_binary{ ref{ pred }, ref{ proj1 }, ref{ proj2 } });
    });
}

template<class Range, class UnaryFunc, class Proj = identity>
auto for_each(Range&& range, UnaryFunc func, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("for_each", range, input_range);

    return std::for_each(std::begin(range), std::end(range), fn(ref{ proj }, ref{ func }));
}

template<class Range, class Generator>
void generate(Range&& range, Generator generator)
{
    MILLRIND_CHECK_CONSTRAINT("generate", range, forward_range);

    std::generate(std::begin(range), std::end(range), std::move(generator));
}

template<class OutputIter, class Size, class Generator>
void generate_n(OutputIter output, Size size, Generator generator)
{
    std::generate_n(output, size, std::move(generator));
}

template<class Range1, class Range2, class Compare = std::less<>, class Proj1 = identity, class Proj2 = identity>
auto includes(Range1&& range1, Range2&& range2, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("includes", range1, input_range);
    MILLRIND_CHECK_CONSTRAINT("includes", range2, input_range);

    return std::includes(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        detail::invoke_binary{ ref{ compare }, ref{ proj1 }, ref{ proj2 } });
}

template<class Range, class Output, class BinaryFunc, class Proj = identity>
auto inclusive_scan(Range&& range, Output output, BinaryFunc func, Proj proj = {})
{
    return std::transform_inclusive_scan(std::begin(range), std::end(range), output, ref{ func }, ref{ proj });
}

template<
    class Range1,
    class Range2,
    class T,
    class BinaryFunc1 = std::plus<>,
    class BinaryFunc2 = std::multiplies<>,
    class Proj1 = identity,
    class Proj2 = identity>
auto inner_product(
    Range1&& range1,
    Range2&& range2,
    T init,
    BinaryFunc1 func1 = {},
    BinaryFunc2 func2 = {},
    Proj1 proj1 = {},
    Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("inner_product", range1, input_range);
    MILLRIND_CHECK_CONSTRAINT("inner_product", range2, input_range);

    return std::inner_product(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        init,
        ref{ func1 },
        detail::invoke_binary{ ref{ func2 }, ref{ proj1 }, ref{ proj2 } });
}

template<class Range, class T>
void iota(Range&& range, T value)
{
    MILLRIND_CHECK_CONSTRAINT("iota", range, forward_range);

    std::iota(std::begin(range), std::end(range), std::move(value));
}

template<class Range, class Compare = std::less<>, class Proj = identity>
auto is_heap(Range&& range, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("is_heap", range, random_access_range);

    return std::is_heap(std::begin(range), std::end(range), detail::invoke_binary{ ref{ compare }, ref{ proj } });
}

template<class Policy = default_return_policy, class Range, class Compare = std::less<>, class Proj = identity>
decltype(auto) is_heap_until(Range&& range, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("is_heap_until", range, random_access_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::is_heap_until(b, e, detail::invoke_binary{ ref{ compare }, ref{ proj } });
    });
}

template<class Range, class UnaryPred, class Proj = identity>
auto is_partitioned(Range&& range, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("is_partitioned", range, input_range);

    return std::is_partitioned(std::begin(range), std::end(range), fn(ref{ proj }, ref{ pred }));
}

template<class Range1, class Range2, class BinaryPred = std::equal_to<>, class Proj1 = identity, class Proj2 = identity>
auto is_permutation(Range1&& range1, Range2&& range2, BinaryPred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("is_permutation", range1, forward_range);
    MILLRIND_CHECK_CONSTRAINT("is_permutation", range2, forward_range);

    return std::is_permutation(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        detail::invoke_binary{ ref{ pred }, ref{ proj1 }, ref{ proj2 } });
}

template<class Range, class Compare = std::less<>, class Proj = identity>
auto is_sorted(Range&& range, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("is_sorted", range, forward_range);

    return std::is_sorted(std::begin(range), std::end(range), detail::invoke_binary{ ref{ compare }, ref{ proj } });
}

template<class Policy = default_return_policy, class Range, class Compare = std::less<>, class Proj = identity>
decltype(auto) is_sorted_until(Range&& range, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("is_sorted_until", range, forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::is_sorted_until(b, e, detail::invoke_binary{ ref{ compare }, ref{ proj } });
    });
}

template<class Range1, class Range2, class Compare = std::less<>, class Proj1 = identity, class Proj2 = identity>
auto lexicographical_compare(Range1&& range1, Range2&& range2, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("lexicographical_compare", range1, input_range);
    MILLRIND_CHECK_CONSTRAINT("lexicographical_compare", range2, input_range);

    return std::lexicographical_compare(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        detail::invoke_binary{ ref{ compare }, ref{ proj1 }, ref{ proj2 } });
}

template<class Policy = default_return_policy, class Range, class T, class Compare = std::less<>, class Proj = identity>
decltype(auto) lower_bound(Range&& range, const T& value, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("lower_bound", range, forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return detail::lower_bound(b, e, value, ref{ compare }, ref{ proj });
    });
}

template<class Range, class Compare = std::less<>, class Proj = identity>
void make_heap(Range&& range, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("make_heap", range, random_access_range);

    std::make_heap(std::begin(range), std::end(range), detail::invoke_binary{ ref{ compare }, ref{ proj } });
}

template<class Policy = default_return_policy, class Range, class Compare = std::less<>, class Proj = identity>
decltype(auto) max_element(Range&& range, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("max_element", range, forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::max_element(b, e, detail::invoke_binary{ ref{ compare }, ref{ proj } });
    });
}

template<
    class Range1,
    class Range2,
    class OutputIter,
    class Compare = std::less<>,
    class Proj1 = identity,
    class Proj2 = identity>
auto merge(Range1&& range1, Range2&& range2, OutputIter output, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("merge", range1, input_range);
    MILLRIND_CHECK_CONSTRAINT("merge", range2, input_range);

    return std::merge(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        output,
        detail::invoke_binary{ ref{ compare }, ref{ proj1 }, ref{ proj2 } });
}

template<class Range, class Compare = std::less<>, class Proj = identity>
void inplace_merge(Range&& range, iterator_t<Range> middle, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("inplace_merge", range, bidirectional_range);

    std::inplace_merge(
        std::begin(range), middle, std::end(range), detail::invoke_binary{ ref{ compare }, ref{ proj } });
}

template<class Policy = default_return_policy, class Range, class Compare = std::less<>, class Proj = identity>
auto minmax_element(Range&& range, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("minmax_element", range, forward_range);

    static const auto policy = Policy{};
    auto [b, e] = make_range(range);
    auto [min, max] = std::minmax_element(b, e, detail::invoke_binary{ ref{ compare }, ref{ proj } });
    return std::forward_as_tuple(policy(min, b, e), policy(max, b, e));
}

template<class Policy = default_return_policy, class Range, class Compare = std::less<>, class Proj = identity>
decltype(auto) min_element(Range&& range, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("min_element", range, forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::min_element(b, e, detail::invoke_binary{ ref{ compare }, ref{ proj } });
    });
}

template<
    class Policy = default_return_policy,
    class Range1,
    class Range2,
    class BinaryPred = std::equal_to<>,
    class Proj1 = identity,
    class Proj2 = identity>
decltype(auto) mismatch(Range1&& range1, Range2&& range2, BinaryPred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("mismatch", range1, input_range);
    MILLRIND_CHECK_CONSTRAINT("mismatch", range2, input_range);

    static const auto policy = Policy{};
    auto [b1, e1] = make_range(range1);
    auto [b2, e2] = make_range(range2);
    auto [b, e] = std::mismatch(b1, e1, b2, detail::invoke_binary{ ref{ pred }, ref{ proj1 }, ref{ proj2 } });
    return std::forward_as_tuple(policy(b, b1, e1), policy(e, b2, e2));
}

template<class Range, class OutputIter>
auto move(Range&& range, OutputIter output)
{
    MILLRIND_CHECK_CONSTRAINT("move", range, input_range);

    return std::move(std::begin(range), std::end(range), output);
}

template<class Range, class Compare = std::less<>, class Proj = identity>
auto next_permutation(Range&& range, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("next_permuatation", range, bidirectional_range);

    return std::next_permutation(
        std::begin(range), std::end(range), detail::invoke_binary{ ref{ compare }, ref{ proj } });
}

template<class Range, class UnaryPred, class Proj = identity>
auto none_of(Range&& range, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("none_of", range, input_range);

    return std::none_of(std::begin(range), std::end(range), fn(ref{ proj }, ref{ pred }));
}

template<class Range, class Compare = std::less<>, class Proj = identity>
void nth_element(Range&& range, iterator_t<Range> middle, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("nth_element", range, random_access_range);

    std::nth_element(std::begin(range), middle, std::end(range), detail::invoke_binary{ ref{ compare }, ref{ proj } });
}

template<class Range, class Compare = std::less<>, class Proj = identity>
void partial_sort(Range&& range, iterator_t<Range> middle, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("partial_sort", range, random_access_range);

    std::partial_sort(
        std::begin(range), middle, std::end(range), detail::invoke_binary{ ref{ compare }, ref{ proj } });
}
template<
    class Policy = default_return_policy,
    class Range1,
    class Range2,
    class Compare = std::less<>,
    class Proj1 = identity,
    class Proj2 = identity>
decltype(auto) partial_sort_copy(Range1&& range1, Range2&& range2, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("partial_sort_copy", range1, input_range);
    MILLRIND_CHECK_CONSTRAINT("partial_sort_copy", range2, input_range);

    return detail::invoke_algorithm<Policy>(std::begin(range2), std::end(range2), [&](auto b, auto e) {
        return std::partial_sort_copy(
            std::begin(range1),
            std::end(range1),
            b,
            e,
            detail::invoke_binary{ ref{ compare }, ref{ proj1 }, ref{ proj2 } });
    });
}

template<class Range, class OutputIter, class BinaryFunc = std::plus<>, class Proj = identity>
auto partial_sum(Range&& range, OutputIter output, BinaryFunc func = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("partial_sum", range, input_range);

    return detail::partial_sum(std::begin(range), std::end(range), output, ref{ func }, ref{ proj });
}

template<class Policy = default_return_policy, class Range, class UnaryPred, class Proj = identity>
decltype(auto) partition(Range&& range, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("partition", range, forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::partition(b, e, fn(ref{ proj }, ref{ pred }));
    });
}

template<class Range, class OutputIter1, class OutputIter2, class UnaryPred, class Proj = identity>
auto partition_copy(Range&& range, OutputIter1 result_true, OutputIter2 result_false, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("partition_copy", range, input_range);

    return std::partition_copy(
        std::begin(range), std::end(range), result_true, result_false, fn(ref{ proj }, ref{ pred }));
}

template<class Policy = default_return_policy, class Range, class UnaryPred, class Proj = identity>
decltype(auto) stable_partition(Range&& range, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("stable_partition", range, bidirectional_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::stable_partition(b, e, fn(ref{ proj }, ref{ pred }));
    });
}

template<class Range, class Compare = std::less<>, class Proj = identity>
auto prev_permutation(Range&& range, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("prev_permutation", range, bidirectional_range);

    return std::prev_permutation(
        std::begin(range), std::end(range), detail::invoke_binary{ ref{ compare }, ref{ proj } });
}

template<class Range, class Compare = std::less<>, class Proj = identity>
void push_heap(Range&& range, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("push_heap", range, random_access_range);

    std::push_heap(std::begin(range), std::end(range), detail::invoke_binary{ ref{ compare }, ref{ proj } });
}

template<class Range, class T, class BinaryFunc = std::plus<>, class Proj = identity>
auto reduce(Range&& range, T init, BinaryFunc func = {}, Proj proj = {})
{
    return std::transform_reduce(std::begin(range), std::end(range), std::move(init), ref{ func }, ref{ proj });
}

template<class Policy = default_return_policy, class Range, class T, class Proj = identity>
decltype(auto) remove(Range&& range, const T& value, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("remove", range, forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::remove_if(b, e, fn(ref{ proj }, equal_to(ref{ value })));
    });
}

template<class Policy, class Range, class UnaryPred, class Proj = identity>
decltype(auto) remove_if(Range&& range, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("remove_if", range, forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::remove_if(b, e, fn(ref{ proj }, ref{ pred }));
    });
}

template<class Range, class OutputIter, class T, class Proj = identity>
auto remove_copy(Range&& range, OutputIter output, const T& value, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("remove_copy", range, input_range);

    return std::remove_copy_if(std::begin(range), std::end(range), output, fn(ref{ proj }, equal_to(ref{ value })));
}

template<class Range, class OutputIter, class UnaryPred, class Proj = identity>
auto remove_copy_if(Range&& range, OutputIter output, UnaryPred pred, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("remove_copy_if", range, input_range);

    return std::remove_copy_if(std::begin(range), std::end(range), output, fn(ref{ proj }, ref{ pred }));
}

template<class Range, class T1, class T2, class Proj = identity>
void raplace(Range&& range, const T1& old_value, const T2& new_value, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("replace", range, forward_range);

    std::replace_if(std::begin(range), std::end(range), fn(ref{ proj }, equal_to(ref{ old_value })), new_value);
}

template<class Range, class UnaryPred, class T, class Proj = identity>
void raplace_if(Range&& range, UnaryPred pred, const T& new_value, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("replace_if", range, forward_range);

    std::replace_if(std::begin(range), std::end(range), fn(ref{ proj }, ref{ pred }), new_value);
}

template<class Range, class OutputIter, class T1, class T2, class Proj = identity>
auto replace_copy(Range&& range, OutputIter output, const T1& old_value, const T2& new_value, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("replace_copy", range, input_range);

    return std::replace_copy_if(
        std::begin(range), std::end(range), output, fn(ref{ proj }, equal_to(ref{ old_value })), new_value);
}

template<class Range, class OutputIter, class UnaryPred, class T, class Proj = identity>
auto replace_copy_if(Range&& range, OutputIter output, UnaryPred pred, const T& new_value, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("replace_copy_if", range, input_range);

    return std::replace_copy_if(std::begin(range), std::end(range), output, fn(ref{ proj }, ref{ pred }), new_value);
}

template<class Range>
void reverse(Range&& range)
{
    MILLRIND_CHECK_CONSTRAINT("reverse", range, bidirectional_range);

    std::reverse(std::begin(range), std::end(range));
}

template<class Range, class OutputIter>
auto reverse_copy(Range&& range, OutputIter output)
{
    MILLRIND_CHECK_CONSTRAINT("reverse_copy", range, bidirectional_range);

    return std::reverse_copy(std::begin(range), std::end(range), output);
}

template<class Policy = default_return_policy, class Range>
decltype(auto) rotate(Range&& range, iterator_t<Range> middle)
{
    MILLRIND_CHECK_CONSTRAINT("forward", range, forward_range);

    return detail::invoke_algorithm<Policy>(
        std::begin(range), std::end(range), [&](auto b, auto e) { return std::rotate(b, middle, e); });
}

template<class Range, class OutputIter>
auto rotate_copy(Range&& range, iterator_t<Range> middle, OutputIter output)
{
    MILLRIND_CHECK_CONSTRAINT("forward_copy", range, forward_range);

    return std::rotate_copy(std::begin(range), middle, std::end(range), output);
}

template<
    class Policy = default_return_policy,
    class Range1,
    class Range2,
    class BinaryPred = std::equal_to<>,
    class Proj1 = identity,
    class Proj2 = identity,
    class = forward_range<Range2>>
decltype(auto) search(Range1&& range1, Range2&& range2, BinaryPred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("search", range1, forward_range);
    MILLRIND_CHECK_CONSTRAINT("search", range2, forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range1), std::end(range1), [&](auto b, auto e) {
        return std::search(
            b,
            e,
            std::begin(range2),
            std::end(range2),
            detail::invoke_binary{ ref{ pred }, ref{ proj1 }, ref{ proj2 } });
    });
}

template<class Range, class BinaryPred = std::equal_to<>>
auto default_searcher(Range&& range, BinaryPred pred = {})
{
    MILLRIND_CHECK_CONSTRAINT("default_searcher", range, forward_range);

    return std::default_searcher{ std::begin(range), std::end(range), std::move(pred) };
}

template<class Range, class Hash = std::hash<range_value_t<Range>>, class BinaryPred = std::equal_to<>>
auto boyer_moore_searcher(Range&& range, Hash hash = {}, BinaryPred pred = {})
{
    MILLRIND_CHECK_CONSTRAINT("boyer_moore_searcher", range, random_access_range);

    return std::boyer_moore_searcher{ std::begin(range), std::end(range), std::move(hash), std::move(pred) };
}

template<class Range, class Hash = std::hash<range_value_t<Range>>, class BinaryPred = std::equal_to<>>
auto boyer_moore_horspool_searcher(Range&& range, Hash hash = {}, BinaryPred pred = {})
{
    MILLRIND_CHECK_CONSTRAINT("boyer_moore_horspool_searcher", range, random_access_range);

    return std::boyer_moore_horspool_searcher{ std::begin(range), std::end(range), std::move(hash), std::move(pred) };
}

template<
    class Policy = default_return_policy,
    class Range,
    class Searcher,
    class = decltype(std::declval<Searcher>()(std::declval<iterator_t<Range>>(), std::declval<iterator_t<Range>>()))>
decltype(auto) search(Range&& range, const Searcher& searcher)
{
    MILLRIND_CHECK_CONSTRAINT("search", range, forward_range);

    return detail::invoke_algorithm<Policy>(
        std::begin(range), std::end(range), [&](auto b, auto e) { return std::search(b, e, searcher); });
}

template<
    class Policy = default_return_policy,
    class Range,
    class Size,
    class T,
    class BinaryPred = std::equal_to<>,
    class Proj = identity>
decltype(auto) search_n(Range&& range, Size size, const T& value, BinaryPred pred = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("search_n", range, forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::search_n(b, e, size, value, detail::invoke_binary{ ref{ pred }, ref{ proj } });
    });
}

template<
    class Range1,
    class Range2,
    class OutputIter,
    class Compare = std::less<>,
    class Proj1 = identity,
    class Proj2 = identity>
auto set_difference(
    Range1&& range1, Range2&& range2, OutputIter output, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("set_difference", range1, input_range);
    MILLRIND_CHECK_CONSTRAINT("set_difference", range2, input_range);

    return std::set_difference(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        output,
        detail::invoke_binary{ ref{ compare }, ref{ proj1 }, ref{ proj2 } });
}

template<
    class Range1,
    class Range2,
    class OutputIter,
    class Compare = std::less<>,
    class Proj1 = identity,
    class Proj2 = identity>
auto set_intersection(
    Range1&& range1, Range2&& range2, OutputIter output, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("set_intersection", range1, input_range);
    MILLRIND_CHECK_CONSTRAINT("set_intersection", range2, input_range);

    return std::set_intersection(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        output,
        detail::invoke_binary{ ref{ compare }, ref{ proj1 }, ref{ proj2 } });
}

template<
    class Range1,
    class Range2,
    class OutputIter,
    class Compare = std::less<>,
    class Proj1 = identity,
    class Proj2 = identity>
auto set_symmetric_difference(
    Range1&& range1, Range2&& range2, OutputIter output, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("set_symmetric_difference", range1, input_range);
    MILLRIND_CHECK_CONSTRAINT("set_symmetric_difference", range2, input_range);

    return std::set_symmetric_difference(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        output,
        detail::invoke_binary{ ref{ compare }, ref{ proj1 }, ref{ proj2 } });
}

template<
    class Range1,
    class Range2,
    class OutputIter,
    class Compare = std::less<>,
    class Proj1 = identity,
    class Proj2 = identity>
auto set_union(Range1&& range1, Range2&& range2, OutputIter output, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("set_union", range1, input_range);
    MILLRIND_CHECK_CONSTRAINT("set_union", range2, input_range);

    return std::set_union(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        output,
        detail::invoke_binary{ ref{ compare }, ref{ proj1 }, ref{ proj2 } });
}

template<class Range, class RandomNumberGenerator>
void shuffle(Range&& range, RandomNumberGenerator generator)
{
    MILLRIND_CHECK_CONSTRAINT("shuffle", range, random_access_range);

    std::shuffle(std::begin(range), std::end(range), std::move(generator));
}

template<class Range, class Compare = std::less<>, class Proj = identity>
void sort(Range&& range, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("sort", range, random_access_range);

    std::sort(std::begin(range), std::end(range), detail::invoke_binary{ ref{ compare }, ref{ proj } });
}

template<class Range, class Compare = std::less<>, class Proj = identity>
void stable_sort(Range&& range, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("stable_sort", range, random_access_range);

    std::stable_sort(std::begin(range), std::end(range), detail::invoke_binary{ ref{ compare }, ref{ proj } });
}

template<class Range, class OutputIter, class UnaryFunc, class Proj = identity>
auto transform(Range&& range, OutputIter output, UnaryFunc func, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("transform", range, input_range);

    return std::transform(std::begin(range), std::end(range), output, fn(ref{ proj }, ref{ func }));
}

template<
    class Range1,
    class Range2,
    class OutputIter,
    class BinaryFunc,
    class Proj1 = identity,
    class Proj2 = identity,
    class = output_iterator<OutputIter>>
auto transform(Range1&& range1, Range2&& range2, OutputIter output, BinaryFunc func, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    MILLRIND_CHECK_CONSTRAINT("transform", range1, input_range);
    MILLRIND_CHECK_CONSTRAINT("transform", range2, input_range);

    return std::transform(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        output,
        detail::invoke_binary{ ref{ func }, ref{ proj1 }, ref{ proj2 } });
}

template<class Range, class Output, class T, class BinaryFunc, class UnaryFunc, class Proj = identity>
auto transform_exclusive_scan(Range&& range, Output output, T init, BinaryFunc func, UnaryFunc op, Proj proj = {})
{
    return std::transform_exclusive_scan(std::begin(range), std::end(range), output, init, func, fn(ref{ proj }, ref{ op }));
}

template<class Range, class Output, class BinaryFunc, class UnaryFunc, class Proj = identity>
auto transform_inclusive_scan(Range&& range, Output output, BinaryFunc func, UnaryFunc op, Proj proj = {})
{
    return std::transform_inclusive_scan(std::begin(range), std::end(range), output, ref{ func }, fn(ref{ proj }, ref{ op }));
}

template<class Range, class T, class BinaryFunc, class UnaryFunc, class Proj = identity>
auto transform_reduce(Range&& range, T init, BinaryFunc func, UnaryFunc op, Proj proj = {})
{
    return std::transform_reduce(std::begin(range), std::end(range), std::move(init), ref{ func }, fn(ref{ proj }, ref{ op }));
}

template<class Policy = default_return_policy, class Range, class BinaryPred = std::equal_to<>, class Proj = identity>
decltype(auto) unique(Range&& range, BinaryPred pred = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("unique", range, forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::unique(b, e, detail::invoke_binary{ ref{ pred }, ref{ proj } });
    });
}

template<class Range, class OutputIter, class BinaryPred = std::equal_to<>, class Proj = identity>
auto unique_copy(Range&& range, OutputIter output, BinaryPred pred = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("unique_copy", range, forward_range);

    return std::unique_copy(
        std::begin(range), std::end(range), output, detail::invoke_binary{ ref{ pred }, ref{ proj } });
}

template<class Policy = default_return_policy, class Range, class T, class Compare = std::less<>, class Proj = identity>
decltype(auto) upper_bound(Range&& range, const T& value, Compare compare = {}, Proj proj = {})
{
    MILLRIND_CHECK_CONSTRAINT("upper_bound", range, forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return detail::upper_bound(b, e, value, ref{ compare }, ref{ proj });
    });
}

}  // namespace millrind
