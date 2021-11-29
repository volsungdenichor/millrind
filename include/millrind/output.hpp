#pragma once

#include <functional>
#include <iostream>
#include <sstream>
#include <string_view>

namespace millrind
{
struct ostream_manipulator
{
    using function_type = std::function<void(std::ostream&)>;

    function_type func;

    friend std::ostream& operator<<(std::ostream& os, const ostream_manipulator& item)
    {
        item.func(os);
        return os;
    }
};

struct ostream_iterator : std::iterator<std::output_iterator_tag, void, void, void, void>
{
    std::ostream* os;
    std::string_view separator;

    ostream_iterator(std::ostream& os, std::string_view separator = {})
        : os{ &os }
        , separator{ separator }
    {
    }

    ostream_iterator& operator*()
    {
        return *this;
    }

    ostream_iterator& operator++()
    {
        return *this;
    }

    ostream_iterator& operator++(int)
    {
        return *this;
    }

    template <class T>
    ostream_iterator& operator=(const T& item)
    {
        *os << item << separator;
        return *this;
    }
};

namespace detail
{
struct delimit_fn
{
    template <class Range, class Proj = identity>
    auto operator()(Range&& range, std::string_view separator = {}, Proj proj = {}) const -> ostream_manipulator
    {
        return impl(std::begin(range), std::end(range), separator, proj);
    }

private:
    template <class Iter, class Proj = identity>
    auto impl(Iter begin, Iter end, std::string_view separator = {}, Proj proj = {}) const -> ostream_manipulator
    {
        return { [=](std::ostream& os) {
            for (auto it = begin; it != end; ++it)
            {
                if (it != begin)
                    os << separator;
                os << std::invoke(proj, *it);
            }
        } };
    }
};

struct write_fn
{
    template <class... Args>
    std::ostream& operator()(std::ostream& os, Args&&... args) const
    {
        (os << ... << std::forward<Args>(args));
        return os;
    }
};

struct str_fn
{
    template <class... Args>
    std::string operator()(Args&&... args) const
    {
        std::stringstream ss;
        write_fn{}(ss, std::forward<Args>(args)...);
        return ss.str();
    }
};

struct print_fn
{
    std::string_view delimiter = {};

    template <class... Args>
    std::ostream& operator()(Args&&... args) const
    {
        return write_fn{}(std::cout, std::forward<Args>(args)...) << delimiter;
    }
};

}  // namespace detail

static constexpr inline auto delimit = detail::delimit_fn{};
static constexpr inline auto write = detail::write_fn{};
static constexpr inline auto str = detail::str_fn{};
static constexpr inline auto print = detail::print_fn{};
static constexpr inline auto println = detail::print_fn{ "\n" };

}  // namespace millrind
