#pragma once

#include "collect_states.h"
#include "traits.h"

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto isNotEmpty
    = [](const auto& tuple) { return bh::not_(bh::equal(bh::size_c<0>, bh::size(tuple))); };

constexpr auto get_internal_transition_table = [](auto state) {
    return bh::if_(
        has_internal_transition_table(state),
        [](auto parentState) {
            auto internalTransitionTable = parentState.make_internal_transition_table();
            return bh::transform(internalTransitionTable, [parentState](auto internalTransition) {
                return bh::append(
                    bh::prepend(bh::prepend(internalTransition, parentState), parentState),
                    parentState);
            });
        },
        [](auto) { return bh::make_tuple(); })(state);
};

constexpr auto extend_internal_transition = [](auto transition, auto state) {
    return bh::make_tuple(
        bh::at_c<0>(transition),
        state,
        bh::at_c<2>(transition),
        bh::at_c<3>(transition),
        bh::at_c<4>(transition),
        state);
};

constexpr auto flatten_internal_transition_table = [](auto parentState) {
    auto states = collect_states_recursive(parentState);

    auto internalTransitionTables = bh::transform(states, get_internal_transition_table);
    auto internalTransitions = bh::flatten(bh::filter(internalTransitionTables, isNotEmpty));

    return bh::flatten(bh::transform(internalTransitions, [states](auto transition) {
        return bh::transform(states, [transition](auto state) {
            return extend_internal_transition(transition, state);
        });
    }));
};

} // namespace hsm