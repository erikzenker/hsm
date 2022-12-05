#pragma once

#include "hsm/details/resolve_state.h"
#include "hsm/details/switch.h"
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

/**
 * This function iterates recursively trough the transition table of a state provided by
 * `make_transition_table()` and collects all transitions.
 *
 * @return a list of ExtendedTransition
 */
template <class State> constexpr auto flatten_transition_table(State state)
{
    auto flattenSubTransitionTable = [state](auto transition) {
        return bh::prepend(
            flatten_sub_transition_table(resolveSubStateParent(transition)),
            details::extended_transition(state, transition));
    };

    return bh::flatten(bh::transform(make_transition_table2(state), flattenSubTransitionTable));
}

namespace {

template <class State> constexpr auto flatten_sub_transition_table(State state)
{
    // clang-format off
    if constexpr(has_transition_table(state)){
        return flatten_transition_table(state);
    }
    else {
        return bh::make_basic_tuple();
    }
    // clang-format on
}
}
}