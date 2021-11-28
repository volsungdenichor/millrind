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

    pythagorean_triples()
        | seq::take(10)
        | seq::enumerate()
        | seq::drop(3)
        | seq::for_each(
            [](auto&& index, auto&& triple) {
                const auto [x, y, z] = std::move(triple);
                println("[", index, "] ", x, " ", y, " ", z);
            });
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