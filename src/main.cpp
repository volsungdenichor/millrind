#include <bit>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <millrind/algorithm.hpp>
#include <millrind/algorithm_ext.hpp>
#include <millrind/format.hpp>
#include <millrind/macros.hpp>
#include <millrind/opt.hpp>
#include <millrind/output.hpp>
#include <millrind/parse.hpp>
#include <millrind/seq.hpp>
#include <millrind/std_ostream.hpp>
#include <random>
#include <sstream>

template<class Range, class Func>
struct map_view
{
    struct data_t
    {
        Range range;
        Func func;

        data_t(Range range, Func func)
            : range{ std::move(range) }
            , func{ std::move(func) }
        {
        }
    };

    std::shared_ptr<data_t> _data;

    map_view(Range range, Func func)
        : _data{ std::make_shared<data_t>(std::move(range), std::move(func)) }
    {
    }

    using inner_iter = millrind::iterator_t<Range>;

    struct iterator
    {
        std::shared_ptr<data_t> _data;
        inner_iter _iter;

        iterator() = default;

        iterator(std::shared_ptr<data_t> data, inner_iter iter)
            : _data{ std::move(data) }
            , _iter{ iter }
        {
        }

        iterator(const iterator&) = default;

        iterator& operator=(const iterator&) = default;

        iterator& operator++()
        {
            ++_iter;
            return *this;
        }

        iterator& operator--()
        {
            --_iter;
            return *this;
        }

        bool operator==(const iterator& other) const
        {
            return _iter == other._iter;
        }

        bool operator!=(const iterator& other) const
        {
            return _iter != other._iter;
        }

        decltype(auto) operator*() const
        {
            return std::invoke(_data->func, *_iter);
        }
    };

    constexpr auto begin() const
    {
        return iterator{ _data, std::begin(_data->range) };
    }

    constexpr auto end() const
    {
        return iterator{ _data, std::end(_data->range) };
    }
};

template<class Func>
auto views_map(Func func)
{
    return millrind::pipeable_adaptor{
        [=](auto&& item) {
            return map_view{ std::move(item), func };
        }
    };
}

template<class T>
constexpr auto yield_if(bool condition, T value)
{
    return millrind::seq::repeat(std::move(value), condition ? 1 : 0);
}

auto pythagorean_triples() -> millrind::iterable<std::tuple<int, int, int>>
{
    using namespace millrind;

    return seq::iota(1, std::numeric_limits<int>::max())
           | seq::transform_join([](int z) {
                 return seq::iota(1, z + 1)
                        | seq::transform_join([=](int x) {
                              return seq::iota(x, z + 1)
                                     | seq::transform_join([=](int y) {
                                           return yield_if(x * x + y * y == z * z, std::tuple{ x, y, z });
                                       });
                          });
             });
}

void run()
{
    using namespace millrind;

    for (auto&& x : std::vector<int>{ 1, 2, 3, 4 } | views_map(L(_ * 10)))
        println(x);
}

int main()
{
    try
    {
        run();
    }
    catch (std::exception& ex)
    {
        std::cerr << "exception caught: " << ex.what() << std::endl;
    }

    return 0;
}