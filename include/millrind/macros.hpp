#pragma once

#define FWD(...)         std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)
#define LAMBDA(arg, ...) [&](auto&& arg) -> decltype(__VA_ARGS__) { return (__VA_ARGS__); }
#define L(...)           LAMBDA(_, __VA_ARGS__)
#define LIFT(func)       L(func(FWD(_)))