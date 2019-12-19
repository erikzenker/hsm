#pragma once

#include <boost/hana.hpp>

namespace hsm {
    namespace bh {
        using namespace boost::hana;    
    }


constexpr auto otherwise = boost::hana::is_valid([](auto... /*arg*/) { });

template <class Pred, class Func>
auto case_(Pred pred, Func func){
    return boost::hana::make_pair(pred, func);    
}


template <class ...Case>
const auto switch_(Case... cases_){
    auto cases = boost::hana::make_tuple(cases_...);    
    return [cases](auto ...args){
        auto is_true = [&](auto predAndFunc){return boost::hana::first(predAndFunc)(args...);};
        auto trueCase = boost::hana::find_if(cases, is_true);
        auto func = boost::hana::second(trueCase.value());
        return func(args...);
    };
}
}