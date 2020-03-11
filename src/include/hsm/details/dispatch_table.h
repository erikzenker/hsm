#pragma once

#include "flatten_internal_transition_table.h"
#include "switch.h"
#include "for_each_idx.h"
#include "transition_table.h"

#include <boost/hana.hpp>

#include <functional>
#include <optional>
#include <tuple>
#include <vector>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

constexpr auto nParentStates
    = [](auto rootState) { return bh::length(collect_parent_state_typeids(rootState)); };
constexpr auto nStates
    = [](auto rootState) { return bh::length(collect_state_typeids_recursive(rootState)); };
constexpr auto nEvents
    = [](auto rootState) { return bh::length(collect_event_typeids_recursive(rootState)); };

/**
 * Collect the initial states for the parent states
 * and returns it as tuple of state idx.
 * 
 * Returns: [[StateIdx]]
 * 
 * Example: [[0,1], [0], [1], [1,2]]
  */
constexpr auto collect_initial_state_typeids = [](auto rootState, auto parentStateTypeids) {
    return bh::transform(parentStateTypeids, [rootState](auto parentStateTypeid) {
        using ParentState = typename decltype(parentStateTypeid)::type;

        constexpr auto initialStates = ParentState {}.initial_state();
        constexpr auto initialStatesTypeIds
            = bh::transform(initialStates, [rootState](auto initialState) {
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
constexpr auto initialStateSizes = [](auto parentStateTypeids) {
    return bh::transform(parentStateTypeids, [](auto parentStateTypeid) {
        using ParentState = typename decltype(parentStateTypeid)::type;
        return bh::size(ParentState {}.initial_state());
    });
};

/**
 * Returns the maximal number of initial states 
 */
constexpr auto maxInitialStates = [](auto rootState) {
    constexpr auto parentStateTypeids = collect_parent_state_typeids(rootState);
    return bh::fold(initialStateSizes(parentStateTypeids), bh::size_c<0>, bh::max);
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
constexpr auto make_initial_state_map = [](auto rootState) {
    constexpr auto parentStateTypeids = collect_parent_state_typeids(rootState);
    constexpr auto initialStates = collect_initial_state_typeids(rootState, parentStateTypeids);
    return bh::to<bh::map_tag>(to_pairs(bh::zip(parentStateTypeids, initialStates)));
};

constexpr auto fill_initial_state_table = [](auto rootState, auto& initialStateTable) {
    constexpr auto parentStateTypeids = collect_parent_state_typeids(rootState);
    for_each_idx(
        parentStateTypeids,
        [rootState, &initialStateTable](auto parentStateTypeid, auto parentStateId) {
            constexpr auto initialStates
                = bh::find(make_initial_state_map(rootState), parentStateTypeid).value();
            auto initialStatesStateIdx = std::vector<std::size_t>(bh::size(initialStates));

            for_each_idx(initialStates, [&initialStatesStateIdx](auto stateIdx, auto regionId) {
                initialStatesStateIdx[regionId] = stateIdx;
            });

            initialStateTable.at(parentStateId) = initialStatesStateIdx;
        });
};

constexpr auto hasRegions
    = [](auto rootState) { return bh::equal(bh::size_c<1>, maxInitialStates(rootState)); };

template <class... Parameters> struct NextState {
    StateIdx combinedState;
    std::function<bool(Parameters...)> guard;
    std::function<void(Parameters...)> action;
    bool history;
    bool defer;
    bool valid = false;
};

template <StateIdx NStates, class... Parameters>
using DispatchArray = std::array<NextState<Parameters...>, NStates>;

template <StateIdx NStates, class... Parameters> struct DispatchTable {
    static DispatchArray<NStates, Parameters...> table;
};

template <StateIdx NStates, class... Parameters>
DispatchArray<NStates, Parameters...> DispatchTable<NStates, Parameters...>::table {};

constexpr auto resolveDst = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& dst){
        return switch_(
            // TODO: make multi region capable    
            case_(has_transition_table(dst), [](auto submachine) { return bh::at_c<0>(submachine.initial_state());}),
            case_(is_entry_state(dst),       [](auto entry) { return entry.get_state(); }),
            case_(is_direct_state(dst),      [](auto direct) { return direct.get_state(); }),
            // TODO: make multi region capable 
            case_(is_history_state(dst),     [](auto history) { return bh::at_c<0>(history.get_parent_state().initial_state());}),
            case_(otherwise(),               [](auto state) { return state; }))
            (dst);
    },
    getDst(transition));
    // clang-format on
};

constexpr auto resolveDstParent = [](auto transition) {
    // clang-format off
    return bh::apply([](auto&& dst, auto&& transition){
        return switch_(
            case_(has_transition_table(dst), [](auto submachine, auto) { return submachine; }),
            case_(is_entry_state(dst),       [](auto entry, auto) { return entry.get_parent_state(); }),
            case_(is_direct_state(dst),      [](auto direct, auto) { return direct.get_parent_state(); }),
            case_(is_history_state(dst),     [](auto history, auto) { return history.get_parent_state(); }),
            case_(otherwise(),               [](auto, auto transition) { return getSrcParent(transition); }))
            (dst, transition);
    },
    getDst(transition), transition);
    // clang-format on
};

constexpr auto resolveSrc = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& src){
        return switch_(
            case_(is_exit_state(src),   [](auto exit) { return exit.get_state(); }),
            case_(is_direct_state(src), [](auto direct) { return direct.get_state(); }),
            case_(otherwise(),          [](auto state) { return state; }))
            (src);    
    },
    getSrc(transition));
    // clang-format on
};

constexpr auto resolveSrcParent = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& src, auto&& transition){
        return switch_(
            case_(is_exit_state(src),   [](auto exit, auto) { return exit.get_parent_state(); }),
            case_(is_direct_state(src), [](auto direct, auto) { return direct.get_parent_state(); }),
            case_(otherwise(),          [](auto, auto transition) { return getSrcParent(transition); }))
            (src, transition);
    },
    getSrc(transition), transition);
    // clang-format on
};

constexpr auto makeInvalidGuard
    = [](auto dispatchTable) { return decltype(dispatchTable[0].guard) {}; };

constexpr auto makeInvalidAction
    = [](auto dispatchTable) { return decltype(dispatchTable[0].action) {}; };

constexpr auto resolveEntryAction = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& dst){
        return switch_(
            case_(has_entry_action(dst), [](auto dst) { return dst.on_entry(); }),
            case_(otherwise(),           [](auto) { return [](auto...) {}; }))
            (dst);
    },
    getDst(transition));
    // clang-format on
};

constexpr auto resolveExitAction = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& src){
        return switch_(
            case_(has_exit_action(src), [](auto src) { return src.on_exit(); }),
            case_(otherwise(),          [](auto) { return [](auto...) {}; }))
            (src);
    },
    getSrc(transition));
    // clang-format on
};

constexpr auto resolveAction2 = [](auto&& transition) {
    return [exitAction(resolveExitAction(transition)),
            action(getAction(transition)),
            entryAction(resolveEntryAction(transition))](auto... params) {
        exitAction(params...);
        action(params...);
        entryAction(params...);
    };
};

constexpr auto resolveAction = [](auto&& transition, auto&& dispatchTable) {
    // clang-format off                
    return bh::if_(is_no_action(getAction(transition)),
            [](auto, auto dispatchTable) { return makeInvalidAction(dispatchTable); },
            [](auto transition, auto) { return resolveAction2(transition);})
            (transition, dispatchTable);
    // clang-format on
};

constexpr auto resolveGuard = [](auto&& transition, auto&& dispatchTable) {
    // clang-format off
    return bh::apply([](auto&& guard, auto&& dispatchTable){
        return switch_(
            case_(is_no_guard(guard), [](auto, auto dispatchTable) { return makeInvalidGuard(dispatchTable);}),
            case_(otherwise(),        [](auto guard, auto) { return guard;}))
            (guard, dispatchTable);
    },
    getGuard(transition), dispatchTable);
    // clang-format on                   
};


constexpr auto resolveHistory = [](auto&& transition) {
    // clang-format off
    return bh::apply([](auto&& dst){
        return switch_(
            case_(is_history_state(dst), [](auto) { return true; }), 
            case_(otherwise(),           [](auto) { return false; }))
            (dst);
    }, 
    getDst(transition));
    // clang-format on                   
};


constexpr auto addDispatchTableEntry = [](auto combinedStateTypids, auto transition, auto& dispatchTable) {
          const auto from = getCombinedStateIdx(combinedStateTypids, resolveSrcParent(transition), resolveSrc(transition));
          const auto guard = resolveGuard(transition, dispatchTable);
          const auto action = resolveAction(transition, dispatchTable);
          const auto to = getCombinedStateIdx(combinedStateTypids, resolveDstParent(transition), resolveDst(transition));
          const auto history = resolveHistory(transition);
          const auto defer = false;
          const auto valid = true;

          dispatchTable[from] = { to, guard, action, history, defer, valid };
      };

const auto addDispatchTableEntryOfSubMachineExits
    = [](auto combinedStateTypids, auto transition, auto& dispatchTable) {
          bh::if_(
              has_transition_table(getSrc(transition)),
              [&](auto parentState) {
                  auto states = collect_child_states(parentState);

                  bh::for_each(
                      states, [&](auto state) {
                          const auto from = getCombinedStateIdx(combinedStateTypids, parentState, state);
                          const auto guard = getGuard(transition);
                          const auto action = getAction(transition);
                          const auto to = getCombinedStateIdx(
                              combinedStateTypids, resolveDstParent(transition), resolveDst(transition));
                          const auto history = resolveHistory(transition);
                          const auto defer = false;
                          const auto valid = true;

                          dispatchTable[from] = { to, guard, action, history, defer, valid };
                      });
              },
              [](auto) {})(getSrc(transition));
      };

constexpr auto filter_transitions = [](auto transitions, auto eventTypeid) {
    auto isEvent = [eventTypeid](auto transition) {
        return bh::equal(getEvent(transition).typeid_, eventTypeid);
    };

    return bh::filter(transitions, isEvent);
};

template <class RootState, class Transitions, class... Parameters>
constexpr auto fill_dispatch_table_with_transitions(
    RootState rootState, Transitions transitions, Parameters... /*parameters*/)
{
    auto eventTypeids = collect_event_typeids_recursive2(transitions);
    constexpr auto combinedStateTypeids = getCombinedStateTypeids(rootState);
    constexpr StateIdx states = nStates(RootState {}) * nParentStates(RootState {});

    bh::for_each(eventTypeids, [&](auto eventTypeid) {
        using Event = typename decltype(eventTypeid)::type;

        auto filteredTransitions = filter_transitions(transitions, eventTypeid);
        auto& dispatchTable = DispatchTable<states, Event, Parameters...>::table;

        bh::for_each(filteredTransitions, [&](auto transition) {
            addDispatchTableEntry(combinedStateTypeids, transition, dispatchTable);
            addDispatchTableEntryOfSubMachineExits(combinedStateTypeids, transition, dispatchTable);
        });
    });
}

constexpr auto getDeferingTransitions = [](auto rootState) {
    constexpr auto transitionHasDeferedEvents
        = [](auto transition) { return has_deferred_events(getSrc(transition)); };

    constexpr auto transitions = flatten_transition_table(rootState);
    return bh::filter(transitions, transitionHasDeferedEvents);
};

constexpr auto hasDeferedEvents
    = [](auto rootState) { return bh::size(getDeferingTransitions(rootState)); };

template <class RootState, class... Parameters>
constexpr auto
fill_dispatch_table_with_deferred_events(RootState rootState, Parameters... /*parameters*/)
{
    const auto combinedStateTypeids = getCombinedStateTypeids(rootState);
    const auto transitions = getDeferingTransitions(rootState);
    constexpr StateIdx states = nStates(RootState {}) * nParentStates(RootState {});

    bh::for_each(transitions, [&](auto transition) {
        const auto deferredEvents = getSrc(transition).defer_events();

        bh::for_each(deferredEvents, [&](auto event) {
            using Event = decltype(event);

            auto& dispatchTable = DispatchTable<states, Event, Parameters...>::table;
            const auto from = getCombinedStateIdx(
                combinedStateTypeids, resolveSrcParent(transition), resolveSrc(transition));
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