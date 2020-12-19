#pragma once

#include "hsm/details/to_pairs.h"

#include <boost/hana/drop_while.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/minus.hpp>
#include <boost/hana/not_equal.hpp>
#include <boost/hana/range.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/zip.hpp>

//#include <boost/mp11.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class Iterable, class Element>
constexpr auto index_of(Iterable const& iterable, Element const& element)
{
    return bh::apply(
        [](auto size, auto dropped) { return size - dropped; },
        bh::size(iterable),
        bh::size(bh::drop_while(iterable, bh::not_equal.to(element))));
}

template <class Typeids> constexpr auto make_index_map(Typeids typeids)
{
    return bh::apply(
        [](auto typeids, auto range) { return bh::to_map(to_pairs(bh::zip(typeids, range))); },
        typeids,
        bh::to<bh::basic_tuple_tag>(bh::make_range(bh::int_c<0>, bh::size(typeids))));
}

// constexpr auto find = [](auto&& reverseIndexMap, auto index, auto&& closure) {
//     boost::mp11::mp_with_index<bh::size(reverseIndexMap)>(
//         index,
//         [reverseIndexMap, &closure](auto i) { closure(bh::find(reverseIndexMap, i).value()); });
// };

// constexpr auto make_reverse_index_map = [](auto&& tuple) {
//     auto range = bh::to<bh::basic_tuple_tag>(bh::make_range(bh::int_c<0>, bh::size(tuple)));
//     auto ids
//         = bh::transform(range, [](auto&& element) { return boost::mp11::mp_size_t<element> {}; });

//     return bh::to_map(to_pairs(bh::zip(ids, tuple)));
// };
}