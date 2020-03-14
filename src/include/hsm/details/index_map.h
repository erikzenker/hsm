#pragma once

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto index_of = [](auto const& iterable, auto const& element) {
    auto size = decltype(bh::size(iterable)){};
    auto dropped = decltype(bh::size(
        bh::drop_while(iterable, bh::not_equal.to(element))
    )){};
    return size - dropped;
};

constexpr auto to_pairs = [](const auto& tuples) {
    return bh::transform(tuples, [](auto tuple) {
        return bh::make_pair(bh::at_c<0>(tuple), bh::at_c<1>(tuple));
    });
};

const auto make_index_map = [](auto typeids) {
    const auto range = bh::to_tuple(bh::make_range(bh::int_c<0>, bh::size(typeids)));
    return bh::to_map(to_pairs(bh::zip(typeids, range)));
};
}