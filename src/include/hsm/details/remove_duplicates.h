#pragma once

#include <boost/hana/map.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/unique.hpp>
#include <boost/mp11/algorithm.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
constexpr auto to_type_pair = [](auto x) { return bh::make_pair(bh::typeid_(x), x); };
}

template <class Tuple> constexpr auto remove_duplicates(Tuple tuple)
{
    return boost::mp11::mp_unique<std::decay_t<decltype(tuple)>> {};
}

template <class Tuple> constexpr auto remove_duplicate_types(Tuple tuple)
{
    return bh::values(bh::to_map(bh::transform(tuple, to_type_pair)));
}
}