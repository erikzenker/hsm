#pragma once

#include <boost/hana.hpp>

namespace {
constexpr auto function = [](auto&& pair)
{
    return boost::hana::second(pair);
};

constexpr auto predicate = [](auto&& pair)
{
    return boost::hana::first(pair);
};

constexpr auto lazy_predicate
    = [](auto&& pair) { return boost::hana::eval(boost::hana::first(pair)); };
}

namespace hsm {
    namespace bh {
        using namespace boost::hana;    
    }

constexpr auto otherwise = bh::is_valid([](auto... /*arg*/) { });
constexpr auto lazy_otherwise = bh::make_lazy([](auto... /*arg*/) { return bh::true_c; });

constexpr auto case_ = [](auto&& predicate, auto&& function)
{
    return bh::make_pair(predicate, function);
};

constexpr auto switch2 = [](auto&& cases) {
    return bh::apply(
        [](auto&& first, auto&& second) {
            return bh::if_(
                predicate(first),
                [](auto&& first, auto&&) {
                    return [first](auto&&... args) { return function(first)(args...); };
                },
                [](auto&&, auto&& second) {
                    return [second](auto&&... args) { return function(second)(args...); };
                })(first, second);
        },
        bh::at_c<0>(cases),
        bh::at_c<1>(cases));
};

constexpr auto switch_ = [](auto&&... cases_)
{
    return bh::apply([](auto&& cases){
        return [cases](auto&&... args) {
            return function(bh::find_if(cases, predicate).value())(args...);
        };
    }, bh::make_tuple(cases_...));
};

constexpr auto lazy_switch_ = [](auto&&... cases_) {
    return bh::apply(
        [](auto&& cases) {
            return [cases](auto&&... args) {
                return function(bh::find_if(cases, lazy_predicate).value())(args...);
            };
        },
        bh::make_tuple(cases_...));
};
}