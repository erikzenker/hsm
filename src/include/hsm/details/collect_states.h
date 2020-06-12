#pragma once

#include "hsm/details/flatten_transition_table.h"
#include "hsm/details/remove_duplicates.h"
#include "hsm/details/to_pairs.h"
#include "hsm/details/traits.h"

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
        is_initial_state(transition.source()),
        [](auto initial) { return unwrap_typeid(initial).get_state(); },
        [](auto source) { return source; })(transition.source());
};

constexpr auto resolveExtentedInitialState = [](auto transition) {
    return bh::if_(
        is_initial_state(transition.source()),
        [](auto initial) { return unwrap_typeid(initial).get_state(); },
        [](auto source) { return source; })(transition.source());
};

constexpr auto extractExtendedStateTypeids = [](auto transition) {
    return bh::make_basic_tuple(
        bh::typeid_(resolveExtentedInitialState(transition)), bh::typeid_(transition.target()));
};
constexpr auto extractExtendedStates = [](auto transition) {
    return bh::make_basic_tuple(resolveExtentedInitialState(transition), transition.target());
};
const auto extractStates = [](auto transition) {
    return bh::make_basic_tuple(transition.source(), transition.target());
};
constexpr auto extractStateTypeids = [](auto transition) {
    return bh::make_basic_tuple(
        bh::typeid_(resolveInitialState(transition)), bh::typeid_(transition.target()));
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