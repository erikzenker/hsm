#pragma once

#include "flatten_transition_table.h"
#include "remove_duplicates.h"
#include "to_pairs.h"
#include "traits.h"

#include <boost/hana/at.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/flatten.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {

constexpr auto resolveInitialState = [](auto transition) {
    return bh::if_(
        is_initial_state(bh::at_c<0>(transition)),
        [](auto initial) { return unwrap_typeid(initial).get_state(); },
        [](auto source) { return source; })(bh::at_c<0>(transition));
};

constexpr auto resolveExtentedInitialState = [](auto transition) {
    return bh::if_(
        is_initial_state(bh::at_c<1>(transition)),
        [](auto initial) { return unwrap_typeid(initial).get_state(); },
        [](auto source) { return source; })(bh::at_c<1>(transition));
};

constexpr auto extractExtendedStateTypeids = [](auto transition) {
    return bh::make_basic_tuple(
        bh::typeid_(resolveExtentedInitialState(transition)), bh::typeid_(bh::at_c<5>(transition)));
};
constexpr auto extractExtendedStates = [](auto transition) {
    return bh::make_basic_tuple(resolveExtentedInitialState(transition), bh::at_c<5>(transition));
};
const auto extractStates = [](auto transition) {
    return bh::make_basic_tuple(bh::at_c<0>(transition), bh::at_c<4>(transition));
};
constexpr auto extractStateTypeids = [](auto transition) {
    return bh::make_basic_tuple(
        bh::typeid_(resolveInitialState(transition)), bh::typeid_(bh::at_c<4>(transition)));
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
    auto collectedStates
        = bh::append(collect_child_state_typeids_recursive(parentState), bh::typeid_(parentState));
    return collectedStates;
};

constexpr auto collect_states_recursive = [](auto&& parentState) {
    auto collectedStates = bh::append(collect_child_states_recursive(parentState), parentState);
    return remove_duplicate_types(collectedStates);
};

const auto collect_child_state_typeids = [](auto&& state) {
    auto transitions = unwrap_typeid(state).make_transition_table();
    auto collectedStates = bh::flatten(bh::transform(transitions, extractStateTypeids));

    return remove_duplicate_typeids(collectedStates);
};

constexpr auto collect_child_states = [](auto&& state) {
    return remove_duplicate_types(
        bh::flatten(bh::transform(make_transition_table2(state), extractStates)));
};
}