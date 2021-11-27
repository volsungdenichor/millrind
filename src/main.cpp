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

std::string add_spaces(std::string_view text, std::string_view separator = " ")
{
    using namespace millrind;
    return str(delimit(text, separator));
}

template<class T>
std::string add_quotes(const T& value)
{
    using namespace millrind;
    return str('"', value, '"');
}

void run()
{
    using namespace millrind;

    std::vector<std::string> results = { "1", "2", "3", "x33x" };

    const auto f = seq::map(parse<int>)
                   | seq::for_each(println);

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