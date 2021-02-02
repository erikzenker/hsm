#pragma once

#include "hsm/details/collect_states.h"
#include "hsm/details/for_each_idx.h"
#include "hsm/details/idx.h"

#include <boost/hana/find.hpp>
#include <boost/hana/maximum.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/zip.hpp>

#include <iostream>
#include <vector>

namespace hsm {

/**
 * Collect the initial states for the parent states
 *
 * Returns: [[State]]
 *
 */
template <class State> constexpr auto collect_initial_states(State parentState)
{
    constexpr auto childStates = collect_child_states(parentState);
    constexpr auto initialStates = bh::filter(childStates, is_initial_state);
    return bh::transform(initialStates, [](auto initialState) { return get_state(initialState); });
}

/**
 * Collect the initial states for the parent states
 * and returns it as tuple of combined state idx.
 *
 * Returns: [[StateIdx]]
 *
 * Example: [[0,1], [0], [1], [1,2]]
 */
template <class State, class Typeids>
constexpr auto collect_initial_state_stateidx(State rootState, Typeids parentStateTypeids)
{
    return bh::transform(parentStateTypeids, [rootState](auto parentStateTypeid) {
        using ParentState = typename decltype(parentStateTypeid)::type;

        constexpr auto initialStates = collect_initial_states(ParentState {});

        return bh::transform(initialStates, [rootState](auto initialState) {
            return getCombinedStateIdx(
                getCombinedStateTypeids(rootState), ParentState {}, initialState);
        });
    });
}

/**
 * Return a map from parent state id to inital state ids
 *
 * Returns: (ParentStateIdx -> [StateIdx])
 *
 * Example:
 * [[0 -> [0, 1]],
 *  [1 -> [3, 1]],
 *  [2 -> [0, 2]]]
 */
template <class State> inline constexpr auto make_initial_state_map(State rootState)
{
    constexpr auto parentStateTypeids = collect_parent_state_typeids(rootState);
    constexpr auto initialStates = collect_initial_state_stateidx(rootState, parentStateTypeids);
    return bh::to_map(to_pairs(bh::zip(parentStateTypeids, initialStates)));
}

/**
 * Fills the initial state table with the state idx of the initial
 * states.
 *
 * Parameters:
 *  initialStateTable : [[StateIdx]]
 *  initialStateTable[parentStateIdx].size() is the number regions
 *  in the parent state.
 *  initialStateTable[parentStateIdx][regionIdx] is a state idx.
 */
template <class State, class StateTable>
constexpr auto fill_initial_state_table(State rootState, StateTable& initialStateTable)
{
    constexpr auto parentStateTypeids = collect_parent_state_typeids(rootState);
    for_each_idx(
        parentStateTypeids,
        [rootState, &initialStateTable](auto parentStateTypeid, auto parentStateId) {
            bh::apply(
                [](auto parentStateId, auto& initialStateTable, auto initialStates) {
                    auto initialStatesStateIdx = std::vector<std::size_t>(bh::size(initialStates));

                    for_each_idx(
                        initialStates, [&initialStatesStateIdx](auto stateIdx, auto regionId) {
                            initialStatesStateIdx[regionId] = stateIdx;
                        });

                    initialStateTable.at(parentStateId) = initialStatesStateIdx;
                },
                parentStateId,
                initialStateTable,
                bh::find(make_initial_state_map(rootState), parentStateTypeid).value());
        });
}

/**
 * Returns a tuple of initial state sizes
 *
 * Returns: [std::size_t]
 *
 * Example: [3, 1, 2]
 */
template <class Typeids> constexpr auto initialStateSizes(Typeids parentStateTypeids)
{
    return bh::transform(parentStateTypeids, [](auto parentStateTypeid) {
        using ParentState = typename decltype(parentStateTypeid)::type;
        return bh::size(collect_initial_states(ParentState {}));
    });
}

/**
 * Returns the maximal number of initial states
 */
template <class State> constexpr auto maxInitialStates(State rootState)
{
    return bh::maximum(initialStateSizes(collect_parent_state_typeids(rootState)));
}
}