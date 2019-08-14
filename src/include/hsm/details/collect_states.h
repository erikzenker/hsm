#pragma once

#include "flatten_transition_table.h"
#include "remove_duplicates.h"
#include "traits.h"

#include <boost/hana.hpp>

namespace hsm {

    namespace bh{
        using namespace boost::hana; 
    };

    namespace {
    const auto collectState = [](auto const& states, auto&& transition) {
        return bh::append(
            bh::append(states, bh::typeid_(bh::front(transition))),
            bh::typeid_(bh::back(transition)));
    };
    }

    const auto collect_child_states_recursive = [](auto&& parentState) {
        auto transitions = flatten_transition_table(parentState);
        auto collectedStates = bh::fold_left(transitions, bh::make_tuple(), collectState);
        return remove_duplicate_typeids(collectedStates);
    };

    const auto collect_states_recursive = [](auto&& parentState) {
        auto collectedStates
            = bh::append(collect_child_states_recursive(parentState), bh::typeid_(parentState));
        return remove_duplicate_typeids(collectedStates);
    };

    const auto collect_child_states = [](auto&& state) {
        auto transitions = state.make_transition_table();
        auto collectedStates = bh::fold_left(transitions, bh::make_tuple(), collectState);

        return remove_duplicate_typeids(collectedStates);
    };
}