#pragma once

#include <boost/hana/tuple.hpp>

namespace hsm {
namespace bh {
using namespace boost::hana;
}
constexpr auto transition_table = bh::make_basic_tuple;

template <class... Events> constexpr auto events = bh::tuple_t<Events...>;
}