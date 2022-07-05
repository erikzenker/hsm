#pragma once

#include "hsm/details/collect_events.h"
#include "hsm/details/collect_initial_states.h"
#include "hsm/details/dispatch_table.h"
#include "hsm/details/flatten_internal_transition_table.h"
#include "hsm/details/for_each_idx.h"
#include "hsm/details/idx.h"
#include "hsm/details/resolve_state.h"
#include "hsm/details/to_pairs.h"

#include <boost/hana/at_key.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/filter.hpp>
#include <boost/hana/find.hpp>
#include <boost/hana/fold.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/functional/apply.hpp>
#include <boost/hana/functional/capture.hpp>
#include <boost/hana/greater.hpp>
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

template <class State> constexpr decltype(auto) hasParallelRegions(State rootState)
{
    return bh::greater(maxInitialStates(rootState), bh::size_c<1>);
}

template <
    class State,
    class DispatchTables,
    class TransitionTuple,
    class EventTypeid,
    class StatesMap,
    class Dependencies>
constexpr auto addDispatchTableEntry(
    State rootState,
    DispatchTables& dispatchTables,
    TransitionTuple&& transition,
    EventTypeid eventTypeid,
    StatesMap&& statesMap,
    Dependencies optionalDependency)
{
    bh::apply(
        [=, &dispatchTables](auto combinedStateIds, auto source, auto target) {
            bh::apply(
                [=, &dispatchTables](
                    auto fromIdx, auto toIdx, auto history, auto mappedSource, auto mappedTarget)
                    -> void {
                    bh::apply(
                        [=](auto& dispatchTable, auto&& transition2, bool internal) -> void {
                            const auto defer = false;
                            const auto valid = true;

                            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                            dispatchTable[fromIdx].push_front(
                                { toIdx, history, defer, valid, internal, std::move(transition2) });
                        },
                        dispatchTables[eventTypeid],
                        make_transition(
                            transition,
                            resolveAction(transition),
                            transition.guard(),
                            eventTypeid,
                            mappedSource,
                            mappedTarget,
                            optionalDependency),
                        transition.internal());
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
    class DispatchTables,
    class TransitionTuple,
    class EventTypeid,
    class StatesMap,
    class Dependencies>
constexpr auto addDispatchTableEntryOfSubMachineExits(
    State rootState,
    DispatchTables& dispatchTables,
    TransitionTuple transition,
    EventTypeid eventTypeid,
    StatesMap&& statesMap,
    Dependencies optionalDependency)
{
    (void)rootState;
    (void)eventTypeid;
    (void)optionalDependency;

    if constexpr (transition.internal()) {
        return;
    }

    const auto parentState = transition.source();
    if constexpr (has_transition_table(parentState)) {
        bh::for_each(collect_child_state_typeids(parentState), [=, &dispatchTables](auto state) {
            bh::apply(
                [=, &dispatchTables](auto combinedStateTypeids, auto target) {
                    bh::apply(
                        [=, &dispatchTables](
                            auto fromIdx,
                            auto toIdx,
                            auto history,
                            auto mappedParent,
                            auto mappedTarget) {
                            bh::apply(
                                [=](auto& dispatchTable, auto&& transition2, bool internal) {
                                    const auto defer = false;
                                    const auto valid = true;
                                    // TODO: Since dispatch table is static, transitions might be
                                    // added twice
                                    dispatchTable[fromIdx].push_front({ toIdx,
                                                                        history,
                                                                        defer,
                                                                        valid,
                                                                        internal,
                                                                        std::move(transition2) });
                                },
                                dispatchTables[eventTypeid],
                                make_transition(
                                    transition,
                                    resolveAction(transition),
                                    transition.guard(),
                                    eventTypeid,
                                    mappedParent,
                                    mappedTarget,
                                    optionalDependency),
                                transition.internal());
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

template <
    class State,
    class DispatchTables,
    class StatesMap,
    class Dependencies,
    class TransitionTuple>
constexpr auto fill_dispatch_table_with_transitions(
    State rootState,
    DispatchTables& dispatchTables,
    StatesMap&& statesMap,
    Dependencies&& optionalDependency,
    TransitionTuple transitions)
{
    auto eventTypeids = collect_event_typeids_recursive_from_transitions(transitions);
    bh::for_each(
        eventTypeids,
        [rootState, &dispatchTables, statesMap, optionalDependency, transitions](auto eventTypeid) {
            auto filteredTransitions = filter_transitions(transitions, eventTypeid);
            bh::for_each(
                filteredTransitions,
                [rootState, &dispatchTables, statesMap, optionalDependency, eventTypeid](
                    auto transition) {
                    addDispatchTableEntry(
                        rootState,
                        dispatchTables,
                        transition,
                        eventTypeid,
                        statesMap,
                        optionalDependency);
                    addDispatchTableEntryOfSubMachineExits(
                        rootState,
                        dispatchTables,
                        transition,
                        eventTypeid,
                        statesMap,
                        optionalDependency);
                });
        });
}

constexpr auto getDeferingTransitions = [](auto rootState) {
    constexpr auto transitionHasDeferedEvents
        = [](auto transition) { return has_deferred_events(resolveExtentedInitialState(transition)); };

    constexpr auto transitions = flatten_transition_table(rootState);
    return bh::filter(transitions, transitionHasDeferedEvents);
};

constexpr auto hasDeferedEvents = [](auto rootState) {
    return bh::not_equal(bh::size_c<0>, bh::size(getDeferingTransitions(rootState)));
};

template <class RootState, class DispatchTables, class OptionalDependency>
constexpr auto fill_dispatch_table_with_deferred_events(
    RootState rootState, DispatchTables& dispatchTables, OptionalDependency /*optionalDependency*/)
{
    const auto transitions = getDeferingTransitions(rootState);
    bh::for_each(transitions, [=, &dispatchTables](auto transition) {
        const auto deferredEvents = get_defer_events(resolveExtentedInitialState(transition));
        bh::for_each(deferredEvents, [=, &dispatchTables](auto event) {
            const auto combinedStateTypeids = getCombinedStateTypeids(rootState);
            auto& dispatchTable = dispatchTables[event];
            const auto from = getCombinedStateIdx(
                combinedStateTypeids, resolveSrcParent(transition), resolveSrc(transition));

            dispatchTable[from].push_front({});
            dispatchTable[from].front().defer = true;
        });
    });
}

template <class RootState, class DispatchTables, class StatesMap, class OptionalDependency>
constexpr auto fill_dispatch_table_with_external_transitions(
    const RootState& rootState,
    DispatchTables& dispatchTables,
    StatesMap&& statesMap,
    OptionalDependency&& optionalDependecy)
{
    fill_dispatch_table_with_transitions(
        rootState,
        dispatchTables,
        statesMap,
        optionalDependecy,
        flatten_transition_table(rootState));
}

template <class RootState, class DispatchTables, class StatesMap, class OptionalDependency>
constexpr auto fill_dispatch_table_with_internal_transitions(
    const RootState& rootState,
    DispatchTables& dispatchTables,
    StatesMap&& statesMap,
    OptionalDependency&& optionalDependecy)
{
    fill_dispatch_table_with_transitions(
        rootState,
        dispatchTables,
        statesMap,
        optionalDependecy,
        flatten_internal_transition_table(rootState));
}
}