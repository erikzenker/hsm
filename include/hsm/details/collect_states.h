#pragma once

#include "hsm/details/flatten_transition_table.h"
#include "hsm/details/remove_duplicates.h"
#include "hsm/details/to_pairs.h"
#include "hsm/details/traits.h"

#include <boost/hana/at.hpp>
#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/flatten.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

namespace {

constexpr auto resolveInitialState = [](auto transition) {
    if constexpr (is_initial_state(transition.source())) {
        return get_state(transition.source());
    } else {
        return transition.source();
    }
};

constexpr auto resolveExtentedInitialState = [](auto transition) {
    if constexpr (is_initial_state(transition.source())) {
        return get_state(transition.source());
    } else {
        return transition.source();
    }
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

template <class State> constexpr auto collect_child_state_typeids_recursive(State parentState)
{
    auto transitions = flatten_transition_table(parentState);
    auto collectedStates = bh::flatten(bh::transform(transitions, extractExtendedStateTypeids));

    return remove_duplicates(collectedStates);
}

template <class State> constexpr auto collect_child_states_recursive(State parentState)
{
    return bh::flatten(bh::transform(flatten_transition_table(parentState), extractExtendedStates));
    ;
}

template <class State> constexpr auto collect_state_typeids_recursive(State parentState)
{
    auto collectedStates
        = bh::append(collect_child_state_typeids_recursive(parentState), bh::typeid_(parentState));
    return collectedStates;
}

template <class State> constexpr auto collect_states_recursive(State parentState)
{
    return remove_duplicates(bh::append(collect_child_states_recursive(parentState), parentState));
}

template <class State> constexpr auto collect_child_state_typeids(State state)
{
    auto transitions = make_transition_table2(state);
    auto collectedStates = bh::flatten(bh::transform(transitions, extractStateTypeids));

    return remove_duplicates(collectedStates);
}

template <class State> constexpr auto collect_child_states(State state)
{
    return remove_duplicates(
        bh::flatten(bh::transform(make_transition_table2(state), extractStates)));
}
}