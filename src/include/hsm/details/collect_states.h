#pragma once

#include "traits.h"

#include <boost/hana.hpp>

namespace hsm {

    namespace bh{
        using namespace boost::hana; 
    };

    template<class T>
    constexpr auto collect_sub_states(T&& state);

    const auto remove_duplicates
        = [](auto tuple) { return bh::to<bh::tuple_tag>(bh::to<bh::set_tag>(tuple)); };

    const auto collect_child_states_recursive = [](auto parentState) {
        auto collectedStates = bh::fold_left(
            parentState.make_transition_table(), bh::make_tuple(), [](auto const& acc, auto row) {
                return bh::concat(
                    bh::append(
                        bh::append(acc, bh::typeid_(bh::front(row))), bh::typeid_(bh::back(row))),
                    collect_sub_states(bh::back(row)));
            });

        return remove_duplicates(collectedStates);
    };

    const auto collect_states_recursive = [](auto parentState) {
        auto collectedStates
            = bh::append(collect_child_states_recursive(parentState), bh::typeid_(parentState));

        return remove_duplicates(collectedStates);
    };

    template<class T>
    constexpr auto collect_sub_states(T&& state){
        return bh::if_(has_transition_table(state),
            [](auto& stateWithTransitionTable){ return collect_states_recursive(stateWithTransitionTable);},
            [](auto&){ return bh::make_tuple();})(state);
    };

    const auto collect_child_states = [](auto state) {
        auto collectedStates = bh::fold_left(
            state.make_transition_table(), bh::make_tuple(), [](auto const& states, auto row) {
                return bh::append(
                    bh::append(states, bh::typeid_(bh::front(row))), bh::typeid_(bh::back(row)));
            });

        return remove_duplicates(collectedStates);
    };
}