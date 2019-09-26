#pragma once

#include "switch.h"
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
    = [](const auto& rootState) { return bh::length(collect_parent_states(rootState)); };
constexpr auto nStates
    = [](const auto& rootState) { return bh::length(collect_states_recursive(rootState)); };
constexpr auto nEvents
    = [](const auto& rootState) { return bh::length(collect_event_typeids_recursive(rootState)); };

template <class Event> struct NextState {
    StateIdx parentState;
    StateIdx state;
    std::function<bool(Event)> guard;
    std::function<void(Event)> action;
};

template <class RootState, class Event>
using DispatchArray
    = std::array<std::array<NextState<Event>, nStates(RootState {})>, nParentStates(RootState {})>;

template <class RootState, class Event> struct DispatchTable {
    static DispatchArray<RootState, Event> table;
};

template <class RootState, class Event>
DispatchArray<RootState, Event> DispatchTable<RootState, Event>::table {};

constexpr auto resolveDst = [](const auto& transition) {
    return switch_(
        case_(has_transition_table, [](auto submachine) { return submachine.initial_state(); }),
        case_(is_entry_state, [](auto entry) { return entry.get_state(); }),
        case_(is_direct_state, [](auto direct) { return direct.get_state(); }),
        case_(otherwise, [](auto state) { return state; }))(getDst(transition));
};

constexpr auto resolveDstParent = [](const auto& transition) {
    return switch_(
        case_(has_transition_table, [](auto submachine) { return submachine; }),
        case_(is_entry_state, [](auto entry) { return entry.get_parent_state(); }),
        case_(is_direct_state, [](auto direct) { return direct.get_parent_state(); }),
        case_(otherwise, [&transition](auto) { return getSrcParent(transition); }))(
        getDst(transition));
};

constexpr auto resolveSrc = [](const auto& transition) {
    return switch_(
        case_(is_exit_state, [](auto exit) { return exit.get_state(); }),
        case_(is_direct_state, [](auto direct) { return direct.get_state(); }),
        case_(otherwise, [](auto state) { return state; }))(getSrc(transition));
};

constexpr auto resolveSrcParent = [](const auto& transition) {
    return switch_(
        case_(is_exit_state, [](auto exit) { return exit.get_parent_state(); }),
        case_(is_direct_state, [](auto direct) { return direct.get_parent_state(); }),
        case_(otherwise, [transition](auto) { return getSrcParent(transition); }))(
        getSrc(transition));
};

constexpr auto resolveAction = [](const auto& transition) {
    const auto exitAction = switch_(
        case_(has_exit_action, [](auto src) { return src.on_exit(); }),
        case_(otherwise, [transition](auto) { return [](auto) {}; }))(getSrc(transition));
    const auto action = getAction(transition);
    const auto entryAction = switch_(
        case_(has_entry_action, [](auto dst) { return dst.on_entry(); }),
        case_(otherwise, [transition](auto) { return [](auto) {}; }))(getDst(transition));
    return [exitAction, action, entryAction](const auto& event) {
        exitAction(event);    
        action(event);
        entryAction(event);
    };
};

const auto addDispatchTableEntry
    = [](const auto& rootState, const auto& transition, auto& dispatchTable) {
          const auto fromParent = getParentStateIdx(rootState, resolveSrcParent(transition));
          const auto from = getStateIdx(rootState, resolveSrc(transition));
          const auto guard = getGuard(transition);
          const auto action = resolveAction(transition);
          const auto toParent = getParentStateIdx(rootState, resolveDstParent(transition));
          const auto to = getStateIdx(rootState, resolveDst(transition));

          dispatchTable[fromParent][from] = { toParent, to, guard, action };
      };

const auto addDispatchTableEntryOfSubMachineExits
    = [](const auto& rootState, const auto& transition, auto& dispatchTable) {
          bh::if_(
              has_transition_table(getSrc(transition)),
              [&rootState, &dispatchTable, &transition](auto parentState) {
                  auto states = collect_child_states(parentState);

                  bh::for_each(
                      states, [&rootState, &parentState, &dispatchTable, &transition](auto state) {
                          const auto fromParent = getParentStateIdx(rootState, parentState);
                          const auto from = getStateIdx(rootState, state);
                          const auto guard = getGuard(transition);
                          const auto action = getAction(transition);
                          const auto toParent
                              = getParentStateIdx(rootState, resolveDstParent(transition));
                          const auto to = getStateIdx(rootState, resolveDst(transition));

                          dispatchTable[fromParent][from] = { toParent, to, guard, action };
                      });
              },
              [](auto) {})(getSrc(transition));
      };

constexpr auto filter_transitions = [](const auto& transitions, const auto& eventTypeid) {
    auto isEvent
        = [&eventTypeid](auto elem) { return bh::equal(bh::at_c<2>(elem).typeid_, eventTypeid); };

    return bh::filter(transitions, isEvent);
};

template <class RootState> constexpr auto fill_dispatch_table(const RootState& rootState)
{
    const auto eventTypeids = collect_event_typeids_recursive(rootState);
    const auto transitions = flatten_transition_table(rootState);

    bh::for_each(eventTypeids, [&](auto eventTypeid) {
        const auto filteredTransitions = filter_transitions(transitions, eventTypeid);

        using Event = typename decltype(eventTypeid)::type;

        auto& dispatchTable = DispatchTable<RootState, Event>::table;

        bh::for_each(filteredTransitions, [&rootState, &dispatchTable](const auto& transition) {
            addDispatchTableEntry(rootState, transition, dispatchTable);
            addDispatchTableEntryOfSubMachineExits(rootState, transition, dispatchTable);
        });
    });
}
}