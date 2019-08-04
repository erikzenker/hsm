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

}