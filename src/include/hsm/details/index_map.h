#pragma once

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
};

const auto make_index_map = [](auto tuple) {
    return bh::to<bh::map_tag>(bh::second(bh::fold_left(
        tuple, bh::make_pair(bh::int_c<0>, bh::make_tuple()), [](auto acc, auto element) {
            auto i = bh::first(acc);
            auto tuple = bh::second(acc);
            auto inc = bh::plus(i, bh::int_c<1>);

            return bh::make_pair(inc, bh::append(tuple, bh::make_pair(element, i)));
        })));
};
}