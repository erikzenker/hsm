#pragma once

#include "traits.h"
#include "utils.h"

#include <boost/hana.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
};

template <class T> constexpr auto collect_sub_actions(T&& state);

const auto collect_actions_recursive = [](auto state) {
    auto collectedActions = bh::fold_left(
        state.make_transition_table(), bh::make_tuple(), [](auto actions, auto row) {
            return bh::concat(
                bh::append(actions, bh::typeid_(bh::at_c<3>(row))),
                collect_sub_actions(bh::back(row)));
        });
    return remove_duplicates(collectedActions);
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

// template <class T> constexpr auto call_sub_actions(T&& state);

const auto call_actions_recursive = [](auto state) {
    bh::for_each(state.make_transition_table(), [](auto row) {
        auto action = bh::at_c<3>(row);
        action();
    });
};

// template <class T> constexpr auto call_sub_actions(T&& state)
// {
//     return bh::if_(
//         has_transition_table(state),
//         [](auto& stateWithTransitionTable) {
//             return call_actions_recursive(stateWithTransitionTable);
//         },
//         [](auto&) { return bh::make_tuple(); })(state);
// };

}