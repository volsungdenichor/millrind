#pragma once

#include <memory>

#include "../iterator_facade.hpp"
#include "default_constructible_func.hpp"

namespace millrind
{
template<class T>
class any_iterator : public iterator_facade<any_iterator<T>>
{
public:
    struct impl_base
    {
        virtual ~impl_base() = default;
        virtual std::unique_ptr<impl_base> clone() const = 0;
        virtual void inc() = 0;
        virtual T get() const = 0;
        virtual bool is_equal(const impl_base& other) const = 0;
    };

    template<class Iter>
    struct implementation : impl_base
    {
        implementation(Iter iter)
            : _iter{ std::move(iter) }
        {
        }

        std::unique_ptr<impl_base> clone() const override
        {
            return std::make_unique<implementation>(_iter);
        }

        void inc() override
        {
            ++_iter;
        }

        T get() const override
        {
            return *_iter;
        }

        bool is_equal(const impl_base& other) const override
        {
            return _iter == static_cast<const implementation&>(other)._iter;
        }

        Iter _iter;
    };

    any_iterator() = default;

    template<class Iter>
    any_iterator(Iter iter)
        : _impl{ std::make_unique<implementation<Iter>>(iter) }
    {
    }

    any_iterator(const any_iterator& other)
        : _impl{ other._impl ? other._impl->clone() : nullptr }
    {
    }

    any_iterator(any_iterator&&) = default;

    any_iterator& operator=(any_iterator other)
    {
        std::swap(_impl, other._impl);
        return *this;
    }

    void inc()
    {
        _impl->inc();
    }

    T deref() const
    {
        return _impl->get();
    }

    bool is_equal(const any_iterator& other) const
    {
        return (!_impl && !other._impl) || _impl->is_equal(*other._impl);
    }

private:
    std::unique_ptr<impl_base> _impl;
};

}  // namespace millrind

MILLRIND_ITERATOR_TRAITS(::millrind::any_iterator)