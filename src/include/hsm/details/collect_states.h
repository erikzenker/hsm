#pragma once

#include "traits.h"
#include "remove_duplicates.h"

#include <boost/hana.hpp>

namespace hsm {

    namespace bh{
        using namespace boost::hana; 
    };

    template<class T>
    constexpr auto collect_sub_states(T&& state);

    const auto collect_transition_states = [](auto&& transition) {
        return bh::make_tuple(
            bh::typeid_(bh::front(transition)), bh::typeid_(bh::back(transition)));
    };

    const auto collect_child_states_recursive = [](auto&& parentState) {
        auto transitions = parentState.make_transition_table();

        auto collect = [](auto const& states, auto&& transition) {
            return bh::concat(
                bh::concat(states, collect_transition_states(transition)),
                collect_sub_states(bh::back(transition)));
        };

        auto collectedStates = bh::fold_left(transitions, bh::make_tuple(), collect);

        return remove_duplicate_typeids(collectedStates);
    };

    const auto collect_states_recursive = [](auto&& parentState) {
        auto collectedStates
            = bh::append(collect_child_states_recursive(parentState), bh::typeid_(parentState));

        return remove_duplicate_typeids(collectedStates);
    };

    template<class T>
    constexpr auto collect_sub_states(T&& state){
        return bh::if_(has_transition_table(state),
            [](auto& stateWithTransitionTable){ return collect_states_recursive(stateWithTransitionTable);},
            [](auto&){ return bh::make_tuple();})(state);
    };

    const auto collect_child_states = [](auto&& state) {
        auto transitions = state.make_transition_table();

        auto collect = [](auto const& states, auto&& transition) {
            return bh::concat(states, collect_transition_states(transition));
        };

        auto collectedStates = bh::fold_left(transitions, bh::make_tuple(), collect);

        return remove_duplicate_typeids(collectedStates);
    };
}