#pragma once

#include <boost/hana/map.hpp>
#include <boost/hana/unpack.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto to_map = [](auto&& tupleOfPairs) {
    return bh::unpack(std::forward<decltype(tupleOfPairs)>(tupleOfPairs), [](auto&&... pairs) {
        return bh::make_map(std::forward<decltype(pairs)>(pairs)...);
    });
};
}