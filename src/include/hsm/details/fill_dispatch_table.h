#pragma once

#include "hsm/details/collect_events.h"
#include "hsm/details/collect_initial_states.h"
#include "hsm/details/dispatch_table.h"
#include "hsm/details/flatten_internal_transition_table.h"
#include "hsm/details/for_each_idx.h"
#include "hsm/details/idx.h"
#include "hsm/details/resolve_state.h"
#include "hsm/details/to_pairs.h"

#include <boost/hana/equal.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/find.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/functional/apply.hpp>
#include <boost/hana/functional/capture.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/length.hpp>
#include <boost/hana/mult.hpp>
#include <boost/hana/size.hpp>
#include <boost/hana/type.hpp>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class State> constexpr auto nParentStates(State rootState)
{
    return bh::length(collect_parent_state_typeids(rootState));
}

template <class State> constexpr auto nStates(State rootState)
{
    return bh::length(collect_state_typeids_recursive(rootState));
}

template <class State> constexpr auto nEvents(State rootState)
{
    return bh::length(collect_event_typeids_recursive(rootState));
}

template <class State> constexpr auto hasRegions(State rootState)
{
    return bh::equal(bh::size_c<1>, maxInitialStates(rootState));
}

template <
    class State,
    class TransitionTuple,
    class EventTypeid,
    class StatesMap,
    class Dependencies>
constexpr auto addDispatchTableEntry(
    State rootState,
    TransitionTuple&& transition,
    EventTypeid eventTypeid,
    StatesMap&& statesMap,
    Dependencies optionalDependency)
{
    bh::apply(
        [=](auto combinedStateIds, auto source, auto target) {
            bh::apply(
                [=](auto fromIdx, auto toIdx, auto history, auto mappedSource, auto mappedTarget) {
                    bh::apply(
                        [=](auto& dispatchTable, auto&& transition2) {
                            const auto defer = false;
                            const auto valid = true;
                            dispatchTable[fromIdx]
                                = { toIdx, history, defer, valid, std::move(transition2) };
                        },
                        get_dispatch_table(rootState, eventTypeid),
                        make_transition(
                            resolveAction(transition),
                            transition.guard(),
                            eventTypeid,
                            mappedSource,
                            mappedTarget,
                            optionalDependency));
                },
                getCombinedStateIdx(combinedStateIds, resolveSrcParent(transition), source),
                getCombinedStateIdx(combinedStateIds, resolveDstParent(transition), target),
                resolveHistory(transition),
                bh::find(statesMap, bh::typeid_(source)).value(),
                bh::find(statesMap, bh::typeid_(target)).value());
        },
        getCombinedStateTypeids(rootState),
        resolveSrc(transition),
        resolveDst(transition));
}

template <
    class State,
    class TransitionTuple,
    class EventTypeid,
    class StatesMap,
    class Dependencies>
constexpr auto addDispatchTableEntryOfSubMachineExits(
    State rootState,
    TransitionTuple transition,
    EventTypeid eventTypeid,
    StatesMap&& statesMap,
    Dependencies optionalDependency)
{
    (void)rootState;
    (void)eventTypeid;
    (void)optionalDependency;

    constexpr auto parentState = transition.source();
    if constexpr (has_transition_table(parentState)) {
        bh::for_each(collect_child_state_typeids(parentState), [=](auto state) {
            bh::apply(
                [=](auto combinedStateTypeids, auto target) {
                    bh::apply(
                        [=](auto fromIdx,
                            auto toIdx,
                            auto history,
                            auto mappedParent,
                            auto mappedTarget) {
                            bh::apply(
                                [=](auto& dispatchTable, auto&& transition2) {
                                    const auto defer = false;
                                    const auto valid = true;
                                    dispatchTable[fromIdx]
                                        = { toIdx, history, defer, valid, std::move(transition2) };
                                },
                                get_dispatch_table(rootState, eventTypeid),
                                make_transition(
                                    resolveAction(transition),
                                    transition.guard(),
                                    eventTypeid,
                                    mappedParent,
                                    mappedTarget,
                                    optionalDependency));
                        },
                        getCombinedStateIdx(combinedStateTypeids, parentState, state),
                        getCombinedStateIdx(
                            combinedStateTypeids, resolveDstParent(transition), target),
                        resolveHistory(transition),
                        bh::find(statesMap, bh::typeid_(parentState)).value(),
                        bh::find(statesMap, bh::typeid_(target)).value());
                },
                getCombinedStateTypeids(rootState),
                resolveDst(transition));
        });
    }
}

constexpr auto filter_transitions = [](auto transitions, auto eventTypeid) {
    auto isEvent = [eventTypeid](auto transition) {
        return bh::equal(transition.event().typeid_, eventTypeid);
    };

    return bh::filter(transitions, isEvent);
};

template <class State, class StatesMap, class Dependencies, class TransitionTuple>
constexpr auto fill_dispatch_table_with_transitions(
    State rootState,
    StatesMap&& statesMap,
    Dependencies&& optionalDependency,
    TransitionTuple transitions)
{
    constexpr auto eventTypeids = collect_event_typeids_recursive_with_transitions(transitions);
    bh::for_each(eventTypeids, [=](auto eventTypeid) {
        constexpr auto filteredTransitions = filter_transitions(transitions, eventTypeid);
        bh::for_each(filteredTransitions, [=](auto transition) {
            addDispatchTableEntry(
                rootState, transition, eventTypeid, statesMap, optionalDependency);
        });
    });
}

// TODO: Rename properly
template <class State, class StatesMap, class Dependencies, class TransitionTuple>
constexpr auto fill_dispatch_table_with_transitions2(
    State rootState,
    StatesMap&& statesMap,
    Dependencies&& optionalDependency,
    TransitionTuple transitions)
{
    auto eventTypeids = collect_event_typeids_recursive_with_transitions(transitions);
    bh::for_each(eventTypeids, [=](auto eventTypeid) {
        auto filteredTransitions = filter_transitions(transitions, eventTypeid);
        bh::for_each(filteredTransitions, [=](auto transition) {
            addDispatchTableEntryOfSubMachineExits(
                rootState, transition, eventTypeid, statesMap, optionalDependency);
        });
    });
}

constexpr auto getDeferingTransitions = [](auto rootState) {
    constexpr auto transitionHasDeferedEvents
        = [](auto transition) { return has_deferred_events(resolveExtentedInitialState(transition)); };

    constexpr auto transitions = flatten_transition_table(rootState);
    return bh::filter(transitions, transitionHasDeferedEvents);
};

constexpr auto hasDeferedEvents
    = [](auto rootState) { return bh::size(getDeferingTransitions(rootState)); };




template <class RootState, class OptionalDependency>
constexpr auto
fill_dispatch_table_with_deferred_events(RootState rootState, OptionalDependency /*optionalDependency*/)
{
    auto transitions = getDeferingTransitions(rootState);
    bh::for_each(transitions, bh::capture(rootState)([](auto rootState, auto transition){
        const auto deferredEvents = get_defer_events(resolveExtentedInitialState(transition));
        bh::for_each(deferredEvents, bh::capture(rootState, transition)([](auto rootState, auto transition, auto event){
            using Event = typename decltype(event)::type;
            auto combinedStateTypeids = getCombinedStateTypeids(rootState);
            constexpr StateIdx states = nStates(rootState) * nParentStates(rootState);
            auto& dispatchTable = DispatchTable<states, Event>::table;
            const auto from = getCombinedStateIdx(
                combinedStateTypeids, resolveSrcParent(transition), resolveSrc(transition));
            dispatchTable[from].defer = true;
        }));
    }));
}

template <class RootState, class StatesMap, class OptionalDependency>
constexpr auto fill_dispatch_table_with_external_transitions(
    const RootState& rootState, StatesMap&& statesMap, OptionalDependency&& optionalDependecy)
{
    fill_dispatch_table_with_transitions(
        rootState, statesMap, optionalDependecy, flatten_transition_table(rootState));
}

template <class RootState, class StatesMap, class OptionalDependency>
constexpr auto fill_dispatch_table_with_submachine_exits(
    const RootState& rootState, StatesMap&& statesMap, OptionalDependency&& optionalDependecy)
{
    fill_dispatch_table_with_transitions2(
        rootState, statesMap, optionalDependecy, flatten_transition_table(rootState));
}

template <class RootState, class StatesMap, class OptionalDependency>
constexpr auto fill_dispatch_table_with_internal_transitions(
    const RootState& rootState, StatesMap&& statesMap, OptionalDependency&& optionalDependecy)
{
    fill_dispatch_table_with_transitions(
        rootState, statesMap, optionalDependecy, flatten_internal_transition_table(rootState));
}
}