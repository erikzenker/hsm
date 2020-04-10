#pragma once

#include "traits.h"

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {
template <class State> constexpr auto flatten_sub_transition_table(State state);
}

template <class State> constexpr auto flatten_transition_table(State state)
{

    const auto flattenSubTransitionTable = [state](auto transition) {
        const auto extentedTransition = bh::prepend(transition, state);
        return bh::prepend(flatten_sub_transition_table(bh::back(transition)), extentedTransition);
    };

    constexpr auto transitionTable = unfold_typeid(state).make_transition_table();
    const auto extendedTransitionTable = bh::transform(transitionTable, flattenSubTransitionTable);
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
        [](auto) { return bh::make_tuple(); })(state);
    // clang-format on
}
}
}