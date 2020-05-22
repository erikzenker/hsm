#pragma once

#include "hsm/details/state.h"

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto transition_table = bh::make_tuple;
constexpr auto transitions = bh::make_tuple;
constexpr auto row = bh::make_tuple;
constexpr auto transition = bh::make_tuple;
constexpr auto events = bh::make_tuple;
}
