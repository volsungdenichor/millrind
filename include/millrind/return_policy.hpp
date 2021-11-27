#pragma once

#include "iterator_range.hpp"
#include "opt.hpp"

namespace millrind
{
struct return_found
{
    template<class Iter>
    constexpr auto operator()(Iter found, Iter, Iter) const -> Iter
    {
        return found;
    }
};

struct return_found_end
{
    template<class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> iterator_range<Iter>
    {
        return make_range(found, end);
    }
};

struct return_begin_found
{
    template<class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> iterator_range<Iter>
    {
        return make_range(begin, found);
    }
};

struct return_found_next
{
    template<class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> iterator_range<Iter>
    {
        return found != end ? make_range(found, std::next(found)) : make_range(found, found);
    }
};

struct return_begin_next
{
    template<class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> iterator_range<Iter>
    {
        return found != end ? make_range(begin, std::next(found)) : make_range(found, found);
    }
};

struct return_next_end
{
    template<class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> iterator_range<Iter>
    {
        return found != end ? make_range(std::next(found), end) : make_range(found, found);
    }
};

struct return_ref
{
    template<class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> iter_reference_t<Iter>
    {
        if (found == end)
            throw std::runtime_error{ "invalid iterator" };
        return *found;
    }
};

struct return_opt_ref
{
    template<class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> std::optional<wrapped<iter_reference_t<Iter>>>
    {
        using result_type = std::optional<wrapped<iter_reference_t<Iter>>>;
        return found != end ? result_type{ *found } : result_type{};
    }
};

struct return_opt_found
{
    template<class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> std::optional<Iter>
    {
        using result_type = std::optional<Iter>;
        return found != end ? result_type{ found } : result_type{};
    }
};

struct return_both
{
    template<class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const
        -> std::pair<iterator_range<Iter>, iterator_range<Iter>>
    {
        return { make_range(begin, found), make_range(found, end) };
    }
};

using default_return_policy = return_found_end;

}  // namespace millrind
