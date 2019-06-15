
#pragma once

#include "traits.h"
#include "remove_duplicates.h"

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
};

template <class T> constexpr auto collect_sub_action_typeids(T&& state);

const auto collect_action_typeids_recursive = [](auto state) {
    auto collectedActions = bh::fold_left(
        state.make_transition_table(), bh::make_tuple(), [](auto actions, auto row) {
            return bh::concat(
                bh::append(actions, bh::typeid_(bh::at_c<3>(row))),
                collect_sub_action_typeids(bh::back(row)));
        });
    return remove_duplicate_typeids(collectedActions);
};

template <class T> constexpr auto collect_sub_action_typeids(T&& state)
{
    return bh::if_(
        has_transition_table(state),
        [](auto& stateWithTransitionTable) {
            return collect_action_typeids_recursive(stateWithTransitionTable);
        },
        [](auto&) { return bh::make_tuple(); })(state);
};

template <class T> constexpr auto collect_sub_actions(T&& state);

const auto collect_actions_recursive = [](auto state) {
    auto collectedActions = bh::fold_left(
        state.make_transition_table(), bh::make_tuple(), [](auto actions, auto row) {
            return bh::concat(
                bh::append(actions, bh::at_c<3>(row)),
                collect_sub_actions(bh::back(row)));
        });
    return remove_duplicate_types(collectedActions);
};

template <class T> constexpr auto collect_sub_actions(T&& state)
{
    return bh::if_(
        has_transition_table(state),
        [](auto& stateWithTransitionTable) {
            return collect_actions_recursive(stateWithTransitionTable);
        },
        [](auto&) { return bh::make_tuple(); })(state);
};


}