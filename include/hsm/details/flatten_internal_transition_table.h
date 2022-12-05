#pragma once

#include "hsm/details/collect_states.h"
#include "hsm/details/traits.h"
#include "hsm/details/transition.h"

#include <boost/hana/append.hpp>
#include <boost/hana/at.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/functional/capture.hpp>
#include <boost/hana/not.hpp>
#include <boost/hana/prepend.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/transform.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

/**
 * Predicate that checks for an empty tuple
 */
constexpr auto isNotEmpty
    = [](const auto& tuple) { return bh::not_(bh::equal(bh::size_c<0>, bh::size(tuple))); };

/**
 * Returns a tuple of internal transitions of a state if it exists.
 * Otherwise a empty tuple is returned. Source and target of the
 * transition are set to parentstate as a placeholder and need to
 * be filled with all child states of the particular state.
 *
 * @param state State for which the internal transitions should be returned
 *
 */
constexpr auto get_internal_transition_table = [](auto parentState) {
    if constexpr (has_internal_transition_table(parentState)) {
        return bh::transform(
            make_internal_transition_table(parentState), [parentState](auto internalTransition) {
                return details::extended_transition(
                    parentState,
                    details::transition(
                        parentState,
                        internalTransition.event(),
                        internalTransition.guard(),
                        internalTransition.action(),
                        parentState));
            });
    } else {
        return bh::make_basic_tuple();
    }
};

/**
 * Extends an internal transitions to all provided states
 *
 * @param internalTranstion Internal transition that should be extended
 * @param states tuple of states
 */
template <class Transition, class States>
constexpr auto extend_internal_transition(Transition internalTransition, States states)
{
    return bh::transform(states, [internalTransition](auto state) {
        return details::internal_extended_transition(
            internalTransition.parent(),
            details::transition(
                state,
                internalTransition.event(),
                internalTransition.guard(),
                internalTransition.action(),
                state));
    });
}

/**
 * Extends an internal transitions to all provided states
 *
 * @param internalTranstion Internal transition that should be extended
 * @param states tuple of states
 */
template <class Transition, class ParentState, class State>
constexpr auto
extend_internal_transition2(Transition internalTransition, ParentState parent, State state)
{
    return bh::make_basic_tuple(details::internal_extended_transition(
        parent,
        details::transition(
            state,
            internalTransition.event(),
            internalTransition.guard(),
            internalTransition.action(),
            state)));
}

/**
 * Returns the internal transitions for each state
 * [[transition1, transition2], [transition3, transition4], []]
 *
 * @param states a tuple of states
 */
constexpr auto get_internal_transitions = [](auto stateAndParentStatePairs) {
    return bh::flatten(bh::filter(
        bh::transform(
            stateAndParentStatePairs,
            [](auto stateAndParentStatePair) {
                constexpr auto extend = bh::capture(stateAndParentStatePair)(
                    [](auto stateAndParentStatePair, auto internalTransition) {
                        // Folowing lines satisfies older gcc -Werror=unused-but-set-parameter
                        (void)internalTransition;
                        if constexpr (has_transition_table(bh::second(stateAndParentStatePair))) {
                            return extend_internal_transition(
                                internalTransition,
                                collect_child_states(bh::second(stateAndParentStatePair)));
                        } else if constexpr (has_internal_transition_table(
                                                 bh::second(stateAndParentStatePair))) {
                            return extend_internal_transition2(
                                internalTransition,
                                bh::first(stateAndParentStatePair),
                                bh::second(stateAndParentStatePair));
                        } else {
                            return bh::make_basic_tuple();
                        }
                    });

                return bh::transform(
                    get_internal_transition_table(bh::second(stateAndParentStatePair)), extend);
            }),
        isNotEmpty));
};

/**
 * Returns a tuple of extended internal transitions reachable from a given rootState
 *
 * @param rootState
 */
template <class State> constexpr auto flatten_internal_transition_table(State rootState)
{
    return [](auto stateAndParentStatePairs) {
        return remove_duplicate_types(bh::to<bh::basic_tuple_tag>(
            bh::flatten(get_internal_transitions(stateAndParentStatePairs))));
    }(collect_state_parentstate_pairs_recursively(rootState));
}

} // namespace hsm