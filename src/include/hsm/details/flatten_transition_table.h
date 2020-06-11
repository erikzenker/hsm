#pragma once

#include "hsm/details/traits.h"
#include "hsm/details/transition.h"

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
    auto flattenSubTransitionTable = [state](auto transition) {
        return bh::prepend(
            flatten_sub_transition_table(transition.target()),
            details::extended_transition(state, transition));
    };

    constexpr auto transitionTable = make_transition_table2(state);
    constexpr auto extendedTransitionTable
        = bh::transform(transitionTable, flattenSubTransitionTable);
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