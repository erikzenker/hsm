#pragma once

#include "traits.h"

#include <boost/hana/back.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/flatten.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/prepend.hpp>
#include <boost/hana/transform.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
template <class State> constexpr auto flatten_sub_transition_table(State state);
}

template <class State> constexpr auto flatten_transition_table(State state)
{
    constexpr auto flattenSubTransitionTable = [](auto state, auto transition) {
        auto extentedTransition = bh::prepend(transition, state);
        return bh::prepend(flatten_sub_transition_table(bh::back(transition)), extentedTransition);
    };

    constexpr auto transitionTable = make_transition_table2(state);
    constexpr auto extendedTransitionTable = bh::transform(transitionTable, bh::partial(flattenSubTransitionTable, state));
    return bh::flatten(extendedTransitionTable);
}

namespace {
template <class State> constexpr auto flatten_sub_transition_table(State state)
{
    // clang-format off
    return bh::if_(
        has_transition_table(state),
        [](auto stateWithTransitionTable) {
            return flatten_transition_table(stateWithTransitionTable);
        },
        [](auto) { return bh::make_basic_tuple(); })(state);
    // clang-format on
}
}
}