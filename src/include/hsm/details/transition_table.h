#pragma once

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}
template <typename... Args> constexpr auto transition_table(Args... args)
{
    return boost::hana::make_tuple(args...);
}

template <typename... Args> constexpr auto row(Args... args)
{
    return boost::hana::make_tuple(args...);
}

template <typename... Args> constexpr auto transition(Args... args)
{
    return boost::hana::make_tuple(args...);
}

constexpr auto getSrcParent = [](auto transition) { return bh::at_c<0>(transition); };

constexpr auto getSrc = [](auto transition) { return bh::at_c<1>(transition); };

constexpr auto getEvent = [](auto transition) { return bh::at_c<2>(transition); };

constexpr auto getGuard = [](auto transition) { return bh::at_c<3>(transition); };

constexpr auto getAction = [](auto transition) { return bh::at_c<4>(transition); };

constexpr auto getDst = [](auto transition) { return bh::at_c<5>(transition); };

const auto is_anonymous_transition
    = [](auto transition) { return bh::typeid_(getEvent(transition)) == bh::typeid_(none {}); };
}