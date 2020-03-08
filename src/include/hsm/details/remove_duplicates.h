#pragma once

#include <boost/hana.hpp>
#include <iostream>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
constexpr auto to_pair = [](auto x) { return bh::make_pair(x, x); };

constexpr auto to_type_pair = [](auto x) { return bh::make_pair(bh::typeid_(x), x); };
}

constexpr auto remove_duplicates = [](auto tuple, auto predicate) {
    return bh::values(bh::to_map(bh::transform(tuple, predicate)));
};

constexpr auto remove_duplicate_typeids
    = [](auto tuple) { return remove_duplicates(tuple, to_pair); };

constexpr auto remove_duplicate_types
    = [](auto tuple) { return remove_duplicates(tuple, to_type_pair); };
}