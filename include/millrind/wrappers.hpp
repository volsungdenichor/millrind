#pragma once
#include <iostream>
#include <type_traits>
#include <utility>

namespace millrind
{
template <class T>
class ref
{
public:
    constexpr explicit ref(T& item) noexcept
        : _ptr{ &item }
    {
    }

    template <class U, class = std::enable_if_t<std::is_constructible_v<T*, U*>>>
    constexpr ref(const ref<U>& other) noexcept
        : ref{ other.get() }
    {
    }

    constexpr ref(const ref&) noexcept = default;

    constexpr ref(ref&&) noexcept = default;

    constexpr ref& operator=(ref other) noexcept
    {
        std::swap(_ptr, other._ptr);
        return *this;
    }

    constexpr T& get() const noexcept
    {
        return *_ptr;
    }

    constexpr operator T&() const
    {
        return get();
    }

    template <class... Args, class = std::enable_if_t<std::is_invocable_v<T, Args...>>>
    constexpr decltype(auto) operator()(Args&&... args) const
    {
        return std::invoke(get(), std::forward<Args>(args)...);
    }

    friend std::ostream& operator<<(std::ostream& os, const ref& item)
    {
        return os << item.get();
    }

private:
    T* _ptr;
};

template <class T>
using cref = ref<const T>;

#define MILLRIND_DEFINE_OP(op)                                       \
    template <class L, class R>                                      \
    constexpr bool operator op(const ref<L>& lhs, const ref<R>& rhs) \
    {                                                                \
        return lhs.get() op rhs.get();                               \
    }

#define MILLRIND_DEFINE_LHS_OP(op)                              \
    template <class L, class R>                                 \
    constexpr bool operator op(const ref<L>& lhs, const R& rhs) \
    {                                                           \
        return lhs.get() op rhs;                                \
    }

#define MILLRIND_DEFINE_RHS_OP(op)                              \
    template <class L, class R>                                 \
    constexpr bool operator op(const L& lhs, const ref<R>& rhs) \
    {                                                           \
        return lhs op rhs.get();                                \
    }

#define MILLRIND_DEFINE_OPERATORS(op) \
    MILLRIND_DEFINE_OP(op)            \
    MILLRIND_DEFINE_LHS_OP(op)        \
    MILLRIND_DEFINE_RHS_OP(op)

MILLRIND_DEFINE_OPERATORS(==)
MILLRIND_DEFINE_OPERATORS(!=)
MILLRIND_DEFINE_OPERATORS(<)
MILLRIND_DEFINE_OPERATORS(<=)
MILLRIND_DEFINE_OPERATORS(>)
MILLRIND_DEFINE_OPERATORS(>=)

#undef MILLRIND_DEFINE_OP
#undef MILLRIND_DEFINE_LHS_OP
#undef MILLRIND_DEFINE_RHS_OP
#undef MILLRIND_DEFINE_OPERATORS

struct wrap_fn
{
    template <class T>
    constexpr auto operator()(T&& item) const -> T&&
    {
        return std::forward<T>(item);
    }

    template <class T>
    constexpr auto operator()(T& item) const -> ref<T>
    {
        return ref<T>{ item };
    }

    template <class T>
    constexpr auto operator()(ref<T> item) const -> ref<T>
    {
        return item;
    }

    template <class T>
    constexpr auto operator()(std::reference_wrapper<T> item) const -> ref<T>
    {
        return ref<T>{ item };
    }
};

struct unwrap_fn
{
    template <class T>
    constexpr auto operator()(T&& item) const -> T&&
    {
        return std::forward<T>(item);
    }

    template <class T>
    constexpr auto operator()(ref<T> item) const -> T&
    {
        return item.get();
    }

    template <class T>
    constexpr auto operator()(std::reference_wrapper<T> item) const -> T&
    {
        return item.get();
    }
};

template <class T>
struct wrapped_impl
{
    using type = T;
};

template <class T>
struct wrapped_impl<T&>
{
    using type = std::reference_wrapper<T>;
};

template <class T>
struct unwrapped_impl
{
    using type = T;
};

template <class T>
struct unwrapped_impl<std::reference_wrapper<T>>
{
    using type = T&;
};

template <class T>
using wrapped = typename wrapped_impl<T>::type;

template <class T>
using unwrapped = typename unwrapped_impl<T>::type;

static constexpr inline auto wrap = wrap_fn{};
static constexpr inline auto unwrap = unwrap_fn{};

}  // namespace millrind

namespace std
{
template <class T>
struct hash<::millrind::ref<T>>
{
    std::size_t operator()(const ::millrind::ref<T>& item) const noexcept
    {
        static const auto inner_hash = std::hash<std::remove_const_t<T>>{};
        return inner_hash(item.get());
    }
};

}  // namespace std