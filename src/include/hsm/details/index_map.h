#pragma once

#include "to_pairs.h"

#include <boost/hana.hpp>
#include <boost/mp11.hpp>

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

const auto make_index_map = [](auto typeids) {
    const auto range = bh::to_tuple(bh::make_range(bh::int_c<0>, bh::size(typeids)));
    return bh::to_map(to_pairs(bh::zip(typeids, range)));
};

// constexpr auto find = [](auto&& reverseIndexMap, auto index, auto&& closure) {
//     boost::mp11::mp_with_index<bh::size(reverseIndexMap)>(
//         index,
//         [reverseIndexMap, &closure](auto i) { closure(bh::find(reverseIndexMap, i).value()); });
// };

// constexpr auto make_reverse_index_map = [](auto&& tuple) {
//     auto range = bh::to_tuple(bh::make_range(bh::int_c<0>, bh::size(tuple)));
//     auto ids
//         = bh::transform(range, [](auto&& element) { return boost::mp11::mp_size_t<element> {}; });

//     return bh::to_map(to_pairs(bh::zip(ids, tuple)));
// };
}