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

void run()
{
    using namespace millrind;

    std::vector<std::string> results = { "1", "2", "3", "x33x", "9" };

    const auto f = tee(L(println(delimit(_, " "))))
                   | seq::adjacent()
                   | seq::for_each([](auto&& a, auto&& b) { println(a, "-", b); });

    f(results);
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