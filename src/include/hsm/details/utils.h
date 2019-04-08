#pragma once

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
};

const auto remove_duplicates
    = [](auto tuple) { return bh::to<bh::tuple_tag>(bh::to<bh::set_tag>(tuple)); };

}