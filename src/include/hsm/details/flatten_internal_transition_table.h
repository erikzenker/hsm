#pragma once

#include "hsm/details/collect_states.h"
#include "hsm/details/traits.h"
#include "hsm/details/transition.h"

#include <boost/hana/append.hpp>
#include <boost/hana/at.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/not.hpp>
#include <boost/hana/prepend.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/transform.hpp>

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
            return bh::transform(
                make_internal_transition_table(parentState),
                [parentState](auto internalTransition) {
                    return details::extended_transition(
                        parentState,
                        details::transition(
                            parentState,
                            internalTransition.event(),
                            internalTransition.guard(),
                            internalTransition.action(),
                            parentState));
                });
        },
        [](auto) { return bh::make_basic_tuple(); })(state);
};

constexpr auto get_internal_transitions = [](auto states) {
    return bh::flatten(
        bh::filter(bh::transform(states, get_internal_transition_table), isNotEmpty));
};

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

template <class State> constexpr auto flatten_internal_transition_table(State parentState)
{
    return [](auto states) {
        constexpr auto extend
            = [states](auto transition) { return extend_internal_transition(transition, states); };

        return bh::to<bh::basic_tuple_tag>(
            bh::flatten(bh::transform(get_internal_transitions(states), extend)));
    }(collect_states_recursive(parentState));
}

} // namespace hsm