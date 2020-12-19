#pragma once

#include <boost/hana/at.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/transform.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}
template <class Tuple> constexpr auto to_pairs(const Tuple& tuple)
{
    return bh::transform(
        tuple, [](auto tuple) { return bh::make_pair(bh::at_c<0>(tuple), bh::at_c<1>(tuple)); });
}
}