#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string_view>

#include "output.hpp"

namespace millrind
{
namespace detail
{
struct format_error : std::runtime_error
{
    format_error(std::string message)
        : std::runtime_error{ std::move(message) }
    {
    }
};

using argument_extractor = std::function<void(std::ostream&, int, std::string_view)>;

inline std::string_view make_string_view(std::string_view::iterator b, std::string_view::iterator e)
{
    if (b < e)
        return { std::addressof(*b), std::string_view::size_type(e - b) };
    else
        return {};
}

inline void apply_format_spec(std::ostream& os, std::string_view fmt)
{
}

inline int parse_int(std::string_view txt)
{
    int result = 0;
    for (char c : txt)
    {
        result = result * 10 + (c - '0');
    }
    return result;
}

template <class T>
void write_arg(std::ostream& os, std::string_view fmt, const T& item)
{
    apply_format_spec(os, fmt);
    os << item;
}

inline void write_args(std::ostream& os, int index, std::string_view fmt)
{
    throw format_error{ "Invalid index" };
}

template <class T, class... Args>
void write_args(std::ostream& os, int index, std::string_view fmt, const T& arg, const Args&... args)
{
    if (index == 0)
        write_arg(os, fmt, arg);
    else
        write_args(os, index - 1, fmt, args...);
}

inline void format_text(std::ostream& os, std::string_view txt)
{
    os << txt;
}

inline void format_arg(std::ostream& os, std::string_view fmt, int arg_index, const argument_extractor& arg_extractor)
{
    const auto colon = std::find(fmt.begin(), fmt.end(), ':');
    const auto index_part = make_string_view(fmt.begin(), colon);
    const auto fmt_part = make_string_view(colon != fmt.end() ? colon + 1 : colon, fmt.end());

    const auto actual_index = !index_part.empty()
                                  ? parse_int(index_part)
                                  : arg_index;

    arg_extractor(os, actual_index, fmt_part);
}

inline void do_format(std::ostream& os, std::string_view fmt, int arg_index, const argument_extractor& arg_extractor)
{
    const auto bracket = std::find_if(fmt.begin(), fmt.end(), [](char c) { return c == '{' || c == '}'; });
    if (bracket == fmt.end())
    {
        return format_text(os, fmt);
    }
    else if (bracket + 1 != fmt.end() && bracket[0] == bracket[1])
    {
        format_text(os, make_string_view(fmt.begin(), bracket + 1));
        return do_format(os, make_string_view(bracket + 2, fmt.end()), arg_index, arg_extractor);
    }
    else if (bracket[0] == '{')
    {
        const auto closing_bracket = std::find(bracket + 1, fmt.end(), '}');
        if (closing_bracket == fmt.end())
        {
            throw format_error{ "unclosed bracket" };
        }
        format_text(os, make_string_view(fmt.begin(), bracket));
        format_arg(os, make_string_view(bracket + 1, closing_bracket), arg_index, arg_extractor);
        return do_format(os, make_string_view(closing_bracket + 1, fmt.end()), arg_index + 1, arg_extractor);
    }
    throw format_error{ "unexpected opening bracket" };
}

template <class... Args>
std::string format(std::string_view fmt, const Args&... args)
{
    const argument_extractor arg_extractor = [&](std::ostream& os, int index, std::string_view f) {
        write_args(os, index, f, args...);
    };

    std::stringstream ss;
    do_format(ss, fmt, 0, arg_extractor);
    return ss.str();
}

struct format_proxy_t
{
    std::string_view fmt;

    template <class... Args>
    std::string operator()(const Args&... args) const
    {
        return format(fmt, args...);
    }
};

}  // namespace detail

using detail::format;

namespace literals
{
inline auto operator""_format(const char* text, std::size_t size) -> detail::format_proxy_t
{
    return { { text, size } };
}

} /* namespace literals */

}  // namespace millrind