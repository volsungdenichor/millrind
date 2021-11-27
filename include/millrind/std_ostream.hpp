#pragma once

#include <optional>
#include <ostream>
#include <string_view>
#include <tuple>
#include <utility>

namespace millrind
{
namespace detail
{
template<class Tuple, size_t... I>
void print_tuple(std::ostream& os, const Tuple& item, std::string_view separator, std::index_sequence<I...>)
{
    (..., (os << (I != 0 ? separator : "") << std::get<I>(item)));
}
}  // namespace detail
}  // namespace millrind

namespace std
{
template<class T>
ostream& operator<<(ostream& os, const optional<T>& item)
{
    if (item)
        return os << "some(" << *item << ")";
    else
        return os << "none";
}

template<class T>
ostream& operator<<(ostream& os, const reference_wrapper<T>& item)
{
    return os << item.get();
}

template<class... Args>
ostream& operator<<(ostream& os, const tuple<Args...>& item)
{
    os << "(";
    ::millrind::detail::print_tuple(os, item, ", ", std::make_index_sequence<sizeof...(Args)>{});
    os << ")";
    return os;
}

template<class T, class U>
ostream& operator<<(ostream& os, const pair<T, U>& item)
{
    os << "(";
    ::millrind::detail::print_tuple(os, item, ", ", std::make_index_sequence<2>{});
    os << ")";
    return os;
}

}  // namespace std
