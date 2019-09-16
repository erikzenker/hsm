#pragma once


#include "traits.h"

#include <boost/hana.hpp>

namespace hsm
{
    
namespace bh{
    using namespace boost::hana;
    }

namespace {
template <class State> constexpr auto flatten_sub_transition_table(State&& state);
}

constexpr auto flatten_transition_table = [](auto state) {
    const auto transitionTable = state.make_transition_table();
    const auto collectedTransitions = bh::fold_left(
        transitionTable, bh::make_tuple(), [state](auto transitions, auto transition) {
            return bh::concat(
                bh::append(transitions, bh::prepend(transition, state)),
                flatten_sub_transition_table(bh::back(transition)));
        });
    return collectedTransitions;
};

namespace {
template <class State> constexpr auto flatten_sub_transition_table(State&& state)
{
    return bh::if_(
        has_transition_table(state),
        [](auto& stateWithTransitionTable) {
            return flatten_transition_table(stateWithTransitionTable);
        },
        [](auto&) { return bh::make_tuple(); })(state);
};
}
} // namespace hsm