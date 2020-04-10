#pragma once

#include "flatten_transition_table.h"
#include "remove_duplicates.h"
#include "traits.h"

#include <boost/hana.hpp>

namespace hsm {

    namespace bh{
        using namespace boost::hana; 
    }

    namespace {
    const auto extractExtendedStateTypeids = [](auto transition) {
        return bh::make_tuple(
            bh::typeid_(bh::at_c<1>(transition)), bh::typeid_(bh::at_c<5>(transition)));
    };
    const auto extractExtendedStates = [](auto transition) {
        return bh::make_tuple(bh::at_c<1>(transition), bh::at_c<5>(transition));
    };
    const auto extractStates = [](auto transition) {
        return bh::make_tuple(
            bh::typeid_(bh::at_c<0>(transition)), bh::typeid_(bh::at_c<4>(transition)));
    };    
    }
    constexpr auto collect_child_state_typeids_recursive = [](auto parentState) {
        auto transitions = flatten_transition_table(parentState);
        auto collectedStates = bh::flatten(bh::transform(transitions, extractExtendedStateTypeids));

        return remove_duplicate_typeids(collectedStates);
    };

    constexpr auto collect_child_states_recursive = [](auto parentState) {
        auto transitions = flatten_transition_table(parentState);
        auto collectedStates = bh::flatten(bh::transform(transitions, extractExtendedStates));

        return collectedStates;
    };

    constexpr auto collect_state_typeids_recursive = [](auto&& parentState) {
        auto collectedStates = bh::append(
            collect_child_state_typeids_recursive(parentState), bh::typeid_(parentState));
        return collectedStates;
    };

    constexpr auto collect_states_recursive = [](auto&& parentState) {
        auto collectedStates = bh::append(collect_child_states_recursive(parentState), parentState);
        return remove_duplicate_types(collectedStates);
    };

    const auto collect_child_states = [](auto&& state) {
        auto transitions = unfold_typeid(state).make_transition_table();
        auto collectedStates = bh::flatten(bh::transform(transitions, extractStates));

        return remove_duplicate_typeids(collectedStates);
    };
}