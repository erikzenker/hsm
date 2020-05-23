#pragma once

#include <boost/hana/eval.hpp>
#include <boost/hana/find_if.hpp>
#include <boost/hana/functional/always.hpp>
#include <boost/hana/lazy.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/functional/compose.hpp>

namespace hsm {
    namespace bh {
        using namespace boost::hana;    
    }

    constexpr auto function = bh::second;
    constexpr auto predicate = bh::first;
    constexpr auto case_ = bh::make_pair;

    constexpr auto otherwise = bh::always(bh::true_c);
    constexpr auto switch_ = [](auto&&... cases_) {
        return function(bh::find_if(bh::make_basic_tuple(cases_...), predicate).value());
    };

    constexpr auto lazy_predicate = bh::compose(bh::eval, bh::first);
    constexpr auto lazy_otherwise = bh::make_lazy(bh::always(bh::true_c));
    constexpr auto lazy_switch_ = [](auto&&... cases_) {
        return function(bh::find_if(bh::make_basic_tuple(cases_...), lazy_predicate).value());
    };
}