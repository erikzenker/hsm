#pragma once

#include <boost/hana.hpp>

namespace hsm {
    namespace bh {
        using namespace boost::hana;    
    }

constexpr auto otherwise = boost::hana::is_valid([](auto... /*arg*/) { });

template <class Pred, class Func> constexpr auto case_(Pred pred, Func func)
{
    return boost::hana::make_pair(pred, func);
}

template <class... Case> constexpr auto switch_(Case... cases_)
{
    constexpr auto cases = boost::hana::make_tuple(cases_...);
    return [cases](auto... args) {
        constexpr auto is_true = [](auto predAndFunc) { return boost::hana::first(predAndFunc); };
        constexpr auto trueCase = boost::hana::find_if(cases, is_true);
        constexpr auto func = boost::hana::second(trueCase.value());
        return func(args...);
    };
}
}