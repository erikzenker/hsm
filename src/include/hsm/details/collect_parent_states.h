#pragma once

#include "traits.h"

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
};

template <class State>
constexpr auto collect_parent_states2(State&& state);

template <class State>
constexpr auto collect_parent_states2(State&& state) {
    return bh::to<bh::tuple_tag>(bh::to<bh::set_tag>(bh::fold_left(
        state.make_transition_table(), bh::make_tuple(), [](auto const& states, auto row) {
            auto subParentStates = bh::if_(
                has_transition_table(bh::back(row)),
                [&states](auto& stateWithTransitionTable) {
                    return bh::append(
                        collect_parent_states2(stateWithTransitionTable),
                        bh::typeid_(stateWithTransitionTable));
                },
                [&states](auto&) { return states; })(bh::back(row));
            return bh::concat(states, subParentStates);
        })));
};

const auto collect_parent_states
    = [](auto state) { return bh::append(collect_parent_states2(state), bh::typeid_(state)); };
}