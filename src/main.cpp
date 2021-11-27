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

std::string func(int a, int b)
{
    return add_quotes(add_spaces(millrind::str(10 * (a + b))));
}

template<class T>
struct try_parse_fn
{
    [[nodiscard]] bool operator()(std::string_view txt, millrind::ref<T> value) const
    {
        std::stringstream ss{ std::string{ txt } };
        ss >> value.get();
        return static_cast<bool>(ss);
    }

    std::optional<T> operator()(std::string_view txt) const
    {
        T temp;
        if ((*this)(txt, millrind::ref{ temp }))
            return std::optional<T>{ std::move(temp) };
        else
            return std::nullopt;
    }
};

template<class T>
static constexpr inline auto try_parse = try_parse_fn<T>{};

int main()
{
    using namespace millrind;

    std::vector<int> results = { 1, 2, 3, 4, 5, 6 };

    std::cout << delimit(find(results, 3), " ") << std::endl;

    return 0;
}