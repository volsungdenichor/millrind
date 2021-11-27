#pragma once

#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include "wrappers.hpp"

namespace millrind
{
namespace detail
{
template<class T>
struct try_parse_fn
{
    [[nodiscard]] bool operator()(std::string_view txt, ref<T> value) const
    {
        std::stringstream ss{ std::string{ txt } };
        ss >> value.get();
        return !ss.fail();
    }

    std::optional<T> operator()(std::string_view txt) const
    {
        T temp;
        if ((*this)(txt, ref{ temp }))
            return std::optional<T>{ std::move(temp) };
        else
            return std::nullopt;
    }
};

template<class T>
struct parse_fn
{
    T operator()(std::string_view txt) const
    {
        if (auto maybe_value = try_parse_fn<T>{}(txt))
        {
            return *maybe_value;
        }
        else
        {
            std::stringstream ss;
            ss << "Cannot parse '" << txt << "'";
            throw std::runtime_error{ ss.str() };
        }
    }
};

}  // namespace detail

template<class T>
static constexpr inline auto try_parse = detail::try_parse_fn<T>{};

template<class T>
static constexpr inline auto parse = detail::parse_fn<T>{};

}  // namespace millrind