#pragma once

#include <boost/hana/at.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/transform.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}
constexpr auto to_pairs = [](const auto& tuples) {
    return bh::transform(
        tuples, [](auto tuple) { return bh::make_pair(bh::at_c<0>(tuple), bh::at_c<1>(tuple)); });
};
}