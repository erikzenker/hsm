#pragma once

#include "flatten_internal_transition_table.h"
#include "switch.h"
#include "for_each_idx.h"
#include "transition_table.h"

#include <boost/hana.hpp>

#include <functional>
#include <optional>
#include <tuple>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto nParentStates
    = [](const auto& rootState) { return bh::length(collect_parent_state_typeids(rootState)); };
constexpr auto nStates
    = [](const auto& rootState) { return bh::length(collect_state_typeids_recursive(rootState)); };
constexpr auto nEvents
    = [](const auto& rootState) { return bh::length(collect_event_typeids_recursive(rootState)); };


/**
 * Collect the initial states for the parent states
 * and returns it as tuple of state idx.
 * 
 * Returns: [[StateIdx]]
 * 
 * Example: [[0,1], [0], [1], [1,2]]
  */
constexpr auto collect_initial_state_typeids = [](const auto& rootState, const auto& parentStateTypeids) {
    return bh::transform(parentStateTypeids, [rootState](auto parentStateTypeid) {
        using ParentState = typename decltype(parentStateTypeid)::type;

        auto initialStates = ParentState {}.initial_state();
        auto initialStatesTypeIds = bh::transform(initialStates, [rootState](auto initialState){
            return getStateIdx(rootState, initialState);
        });

        return initialStatesTypeIds;
    });
};

/**
 * Returns a tuple of initial state sizes
 * 
 * Returns: [std::size_t]
 * 
 * Example: [3, 1, 2]
 */
constexpr auto initialStateSizes = [](const auto& parentStateTypeids) {
    return bh::transform(parentStateTypeids, [](auto parentStateTypeid) {
        using ParentState = typename decltype(parentStateTypeid)::type;
        return bh::size(ParentState {}.initial_state());
    });
};

constexpr auto to_pairs = [](const auto& tuples) {
    return bh::transform(tuples, [](auto tuple) {
        return bh::make_pair(bh::at_c<0>(tuple), bh::at_c<1>(tuple));
    });
};

/**
 * Returns the maximal number of initial states 
 */
constexpr auto maxInitialStates = [](const auto& rootState) {
    auto parentStateTypeids = collect_parent_state_typeids(rootState);

    auto maxInitialStates = bh::fold(
        initialStateSizes(parentStateTypeids), bh::size_c<0>, [](auto currentMax, auto nInitialStates) {
            return bh::max(currentMax, nInitialStates);
        });

    return maxInitialStates;
};

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
constexpr auto make_initial_state_map = [](const auto& rootState) {
    auto parentStateTypeids = collect_parent_state_typeids(rootState);
    auto initialStates = collect_initial_state_typeids(rootState, parentStateTypeids);
    return bh::to<bh::map_tag>(to_pairs(bh::zip(parentStateTypeids, initialStates)));
};

constexpr auto fill_inital_state_table = [](const auto& rootState, auto& initialStateTable) {
    auto parentStateTypeids = collect_parent_state_typeids(rootState);    
    for_each_idx(parentStateTypeids, [rootState, &initialStateTable](auto parentStateTypeid, auto parentStateId){
        auto initialStates = bh::find(make_initial_state_map(rootState), parentStateTypeid).value();
        auto initialStatesStateIdx = std::vector<std::size_t>(bh::size(initialStates));

        for_each_idx(initialStates, [&initialStatesStateIdx](auto stateIdx, auto regionId){
            initialStatesStateIdx[regionId] = stateIdx;
        });

        initialStateTable.at(parentStateId) = initialStatesStateIdx;
    });
};

constexpr auto hasRegions
    = [](const auto& rootState) { return bh::equal(bh::size_c<1>, maxInitialStates(rootState)); };

template <class... Parameters> struct NextState {
    StateIdx combinedState;
    std::function<bool(Parameters...)> guard;
    std::function<void(Parameters...)> action;
    bool history;
    bool defer;
    bool valid = false;
};

template <class RootState, class... Parameters>
using DispatchArray
    = std::array<NextState<Parameters...>, nStates(RootState {}) * nParentStates(RootState {})>;

template <class RootState, class... Parameters> struct DispatchTable {
    static DispatchArray<RootState, Parameters...> table;
};

template <class RootState, class... Parameters>
DispatchArray<RootState, Parameters...> DispatchTable<RootState, Parameters...>::table {};

constexpr auto resolveDst = [](const auto& transition) {
    // clang-format off
    return switch_(
        case_(
            has_transition_table,
            [](auto submachine) { // TODO: make multi region capable
                return bh::at_c<0>(submachine.initial_state());
            }),
        case_(is_entry_state, [](auto entry) { return entry.get_state(); }),
        case_(is_direct_state, [](auto direct) { return direct.get_state(); }),
        case_(
            is_history_state,
            [](auto history) { // TODO: make multi region capable
                return bh::at_c<0>(history.get_parent_state().initial_state());
            }),
        case_(otherwise, [](auto state) { return state; }))(getDst(transition));
    // clang-format on
};

constexpr auto resolveDstParent = [](const auto& transition) {
    // clang-format off
    return switch_(
        case_(has_transition_table, [](auto submachine) { return submachine; }),
        case_(is_entry_state, [](auto entry) { return entry.get_parent_state(); }),
        case_(is_direct_state, [](auto direct) { return direct.get_parent_state(); }),
        case_(is_history_state, [](auto history) { return history.get_parent_state(); }),
        case_(otherwise, [&transition](auto) { return getSrcParent(transition); }))(
        getDst(transition));
    // clang-format on
};

constexpr auto resolveSrc = [](const auto& transition) {
    // clang-format off
    return switch_(
        case_(is_exit_state, [](auto exit) { return exit.get_state(); }),
        case_(is_direct_state, [](auto direct) { return direct.get_state(); }),
        case_(otherwise, [](auto state) { return state; }))(getSrc(transition));
    // clang-format on
};

constexpr auto resolveSrcParent = [](const auto& transition) {
    // clang-format off
    return switch_(
        case_(is_exit_state, [](auto exit) { return exit.get_parent_state(); }),
        case_(is_direct_state, [](auto direct) { return direct.get_parent_state(); }),
        case_(otherwise, [transition](auto) { return getSrcParent(transition); }))(
        getSrc(transition));
    // clang-format on
};

constexpr auto makeInvalidGuard = [](const auto& dispatchTable){
    return decltype(dispatchTable[0].guard) {};
};

constexpr auto makeInvalidAction = [](const auto& dispatchTable){
    return decltype(dispatchTable[0].action) {};
};

constexpr auto resolveAction2 = [](const auto& transition) {
            const auto exitAction = switch_(
                case_(has_exit_action, [](auto src) { return src.on_exit(); }),
                case_(otherwise, [transition](auto) { return [](auto...) {}; }))(
                getSrc(transition));
            const auto action = getAction(transition);
            const auto entryAction = switch_(
                case_(has_entry_action, [](auto dst) { return dst.on_entry(); }),
                case_(otherwise, [transition](auto) { return [](auto...) {}; }))(
                getDst(transition));
            return [exitAction, action, entryAction](const auto&... params) {
                exitAction(params...);
                action(params...);
                entryAction(params...);
            };
};    

constexpr auto resolveAction = [](const auto& transition, const auto& dispatchTable) {
    // clang-format off                
    return bh::if_(is_no_action(getAction(transition)),
            [](auto, auto dispatchTable) { return makeInvalidAction(dispatchTable); },
            [](auto transition, auto) { return resolveAction2(transition);})
            (transition, dispatchTable);
    // clang-format on        
};

constexpr auto resolveGuard = [](const auto& transition, const auto& dispatchTable) {
    // clang-format off            
    return switch_(
             case_(is_no_guard, [&dispatchTable](auto) { return makeInvalidGuard(dispatchTable);}),
             case_(otherwise,   [](auto guard) { return guard;}))
             (getGuard(transition));
    // clang-format on                   
};

constexpr auto resolveHistory = [](const auto& transition) {
    // clang-format off        
    return switch_(
            case_(is_history_state, [](auto) { return true; }), 
            case_(otherwise,        [](auto) {return false;}))
            (getDst(transition));
    // clang-format on                   
};

const auto addDispatchTableEntry
    = [](const auto& rootState, const auto& transition, auto& dispatchTable) {
          const auto from = getCombinedStateIdx(rootState, resolveSrcParent(transition), resolveSrc(transition));
          const auto guard = resolveGuard(transition, dispatchTable);
          const auto action = resolveAction(transition, dispatchTable);
          const auto to = getCombinedStateIdx(
              rootState, resolveDstParent(transition), resolveDst(transition));
          const auto history = resolveHistory(transition);
          const auto defer = false;
          const auto valid = true;

          dispatchTable[from] = { to, guard, action, history, defer, valid };
      };

const auto addDispatchTableEntryOfSubMachineExits
    = [](const auto& rootState, const auto& transition, auto& dispatchTable) {
          bh::if_(
              has_transition_table(getSrc(transition)),
              [&rootState, &dispatchTable, &transition](auto parentState) {
                  auto states = collect_child_states(parentState);

                  bh::for_each(
                      states, [&rootState, &parentState, &dispatchTable, &transition](auto state) {
                          const auto from = getCombinedStateIdx(rootState, parentState, state);
                          const auto guard = getGuard(transition);
                          const auto action = getAction(transition);
                          const auto to = getCombinedStateIdx(
                              rootState, resolveDstParent(transition), resolveDst(transition));
                          const auto history = resolveHistory(transition);
                          const auto defer = false;
                          const auto valid = true;

                          dispatchTable[from] = { to, guard, action, history, defer, valid };
                      });
              },
              [](auto) {})(getSrc(transition));
      };

constexpr auto filter_transitions = [](const auto& transitions, const auto& eventTypeid) {
    auto isEvent = [&eventTypeid](auto transition) {
        return bh::equal(getEvent(transition).typeid_, eventTypeid);
    };

    return bh::filter(transitions, isEvent);
};

template <class RootState, class Transitions, class... Parameters>
constexpr auto fill_dispatch_table_with_transitions(
    const RootState& rootState, const Transitions& transitions, Parameters... /*parameters*/)
{
    const auto eventTypeids = collect_event_typeids_recursive(rootState);

    bh::for_each(eventTypeids, [&](auto eventTypeid) {
        using Event = typename decltype(eventTypeid)::type;

        const auto filteredTransitions = filter_transitions(transitions, eventTypeid);
        auto& dispatchTable = DispatchTable<RootState, Event, Parameters...>::table;

        bh::for_each(filteredTransitions, [&rootState, &dispatchTable](const auto& transition) {
            addDispatchTableEntry(rootState, transition, dispatchTable);
            addDispatchTableEntryOfSubMachineExits(rootState, transition, dispatchTable);
        });
    });
}

constexpr auto getDeferingTransitions = [](const auto& rootState) {
    constexpr auto transitionHasDeferedEvents
        = [](auto transition) { return has_deferred_events(getSrc(transition)); };

    const auto transitions = flatten_transition_table(rootState);
    return bh::filter(transitions, transitionHasDeferedEvents);
};

constexpr auto hasDeferedEvents
    = [](const auto& rootState) { return bh::size(getDeferingTransitions(rootState)); };

template <class RootState, class... Parameters>
constexpr auto
fill_dispatch_table_with_deferred_events(const RootState& rootState, Parameters... /*parameters*/)
{
    const auto transitions = getDeferingTransitions(rootState);
    bh::for_each(transitions, [&](auto transition) {
        auto deferredEvents = getSrc(transition).defer_events();

        bh::for_each(deferredEvents, [&](auto event) {
            using Event = decltype(event);

            auto& dispatchTable = DispatchTable<RootState, Event, Parameters...>::table;
            const auto from = getCombinedStateIdx(
                rootState, resolveSrcParent(transition), resolveSrc(transition));
            dispatchTable[from].defer = true;
        });
    });
}

template <class RootState, class... Parameters>
constexpr auto fill_dispatch_table_with_external_transitions(
    const RootState& rootState, const Parameters&... parameters)
{
    fill_dispatch_table_with_transitions(
        rootState, flatten_transition_table(rootState), parameters...);
}

template <class RootState, class... Parameters>
constexpr auto fill_dispatch_table_with_internal_transitions(
    const RootState& rootState, const Parameters&... parameters)
{
    fill_dispatch_table_with_transitions(
        rootState, flatten_internal_transition_table(rootState), parameters...);
}
}