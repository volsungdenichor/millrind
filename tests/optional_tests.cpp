#include <catch.hpp>
#include <millrind/opt.hpp>
#include <millrind/std_ostream.hpp>

using namespace millrind;

SCENARIO("has_value", "[optional]")
{
    REQUIRE((std::optional{ 1 } | opt::has_value()) == true);
    REQUIRE((std::optional<int>{} | opt::has_value()) == false);
}

SCENARIO("map", "[optional]")
{
    static const auto func = [](int x) { return 10 * x; };
    REQUIRE((std::optional{ 1 } | opt::map(func)) == std::optional{ 10 });
    REQUIRE((std::optional<int>{} | opt::map(func)) == std::nullopt);
}

SCENARIO("take_if", "[optional]")
{
    static const auto pred = [](int x) { return x % 2 == 0; };
    REQUIRE((std::optional{ 2 } | opt::take_if(pred)) == std::optional{ 2 });
    REQUIRE((std::optional{ 3 } | opt::take_if(pred)) == std::nullopt);
}

SCENARIO("drop_if", "[optional]")
{
    static const auto pred = [](int x) { return x % 2 == 0; };
    REQUIRE((std::optional{ 2 } | opt::drop_if(pred)) == std::nullopt);
    REQUIRE((std::optional{ 3 } | opt::drop_if(pred)) == std::optional{ 3 });
}

SCENARIO("flat_map", "[optional]")
{
    static const auto func = [](int x) -> std::optional<int> { return x % 2 == 0 ? std::optional{ 10 * x } : std::nullopt; };
    REQUIRE((std::optional{ 1 } | opt::flat_map(func)) == std::nullopt);
    REQUIRE((std::optional{ 2 } | opt::flat_map(func)) == std::optional{ 20 });
    REQUIRE((std::optional{ 3 } | opt::flat_map(func)) == std::nullopt);
    REQUIRE((std::optional{ 4 } | opt::flat_map(func)) == std::optional{ 40 });
}

SCENARIO("filter map chaining", "[optional]")
{
    static const auto op = opt::filter([](int x) { return x % 2 == 0; }) | opt::map([](int x) { return 10 * x; });
    REQUIRE((std::optional{ 1 } | op) == std::nullopt);
    REQUIRE((std::optional{ 2 } | op) == std::optional{ 20 });
    REQUIRE((std::optional{ 3 } | op) == std::nullopt);
    REQUIRE((std::optional{ 4 } | op) == std::optional{ 40 });
}

SCENARIO("value", "[optional]")
{
    int val = 3;
    int& res = std::optional{ ref{ val } } | opt::value();
    REQUIRE(std::addressof(res) == std::addressof(val));
}

SCENARIO("value_or_throw", "[optional]")
{
    REQUIRE((std::optional{ 1 } | opt::value_or_throw(std::runtime_error{ "X" })) == 1);
    REQUIRE_THROWS_AS((std::optional<int>{} | opt::value_or_throw(std::runtime_error{ "X" })), std::runtime_error);
    REQUIRE_THROWS_AS((std::optional<int>{} | opt::value_or_throw("X")), std::runtime_error);
}

SCENARIO("value_or", "[optional]")
{
    int def = -1;
    int x = 9;
    int& res = std::optional{ ref{ x } } | opt::value_or(ref{ def });
    REQUIRE(std::addressof(res) == std::addressof(x));
}

SCENARIO("value_or (lazy)", "[optional]")
{
    int def = -1;
    int x = 9;
    int& res = std::optional{ ref{ x } } | opt::value_or([&]() { return ref{ def }; });
    REQUIRE(std::addressof(res) == std::addressof(x));
}

SCENARIO("or_", "[optional]")
{
    int x = 3;
    const auto def = []() { return std::optional{ -1 }; };
    REQUIRE((std::optional{ ref{ x } } | opt::or_(std::optional{ 9 })) == std::optional{ 3 });
    REQUIRE((std::optional<int>{} | opt::or_(std::optional{ 9 })) == std::optional{ 9 });
    REQUIRE((std::optional{ 5 } | opt::or_(def)) == std::optional{ 5 });
    REQUIRE((std::optional<int>{} | opt::or_(def)) == std::optional{ -1 });
}

SCENARIO("and_", "[optional]")
{
    int x = 3;
    const auto def = []() { return std::optional{ -1 }; };
    REQUIRE((std::optional{ ref{ x } } | opt::and_(std::optional{ 9 })) == std::optional{ 9 });
    REQUIRE((std::optional<int>{} | opt::and_(std::optional{ 9 })) == std::nullopt);
    REQUIRE((std::optional{ 5 } | opt::and_(def)) == std::optional{ -1 });
    REQUIRE((std::optional<int>{} | opt::and_(def)) == std::nullopt);
}