#pragma once

#include "hsm/details/collect_events.h"
#include "hsm/details/fill_dispatch_table.h"
#include "hsm/details/fill_unexpected_event_handler_tables.h"
#include "hsm/details/idx.h"
#include "hsm/details/make_dispatch_tables.h"
#include "hsm/details/make_states_map.h"
#include "hsm/details/make_unexpected_event_handler_tables.h"
#include "hsm/details/transition_table_traits.h"
#include "hsm/details/variant_queue.h"

#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/contains.hpp>
#include <boost/hana/type.hpp>

#include <array>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <vector>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class RootState, class... OptionalParameters> class sm {
    static constexpr state_t<RootState> rootState {};

    using Region = std::uint8_t;
    using Events = decltype(collect_event_typeids_recursive(rootState));
    using StatesMap = decltype(make_states_map(rootState));
    using DispatchTables = decltype(make_dispatch_tables(rootState));
    using UnexpectedEventHandlerTables = decltype(make_unexpected_event_handler_tables(rootState));
    std::array<StateIdx, maxInitialStates(rootState)> m_currentCombinedState;
    std::array<std::vector<std::size_t>, nParentStates(rootState)> m_initial_states;
    std::array<std::vector<std::size_t>, nParentStates(rootState)> m_history;
    variant_queue<Events> m_defer_queue;
    std::size_t m_currentRegions {};
    StatesMap m_statesMap;
    DispatchTables m_dispatchTables;
    UnexpectedEventHandlerTables m_unexpectedEventHandlerTables;

  public:
    sm(OptionalParameters&... optionalParameters)
        : m_initial_states()
        , m_history()
        , m_defer_queue(collect_event_typeids_recursive(rootState))
        , m_statesMap(make_states_map(rootState))
    {
        static_assert(
            has_transition_table(rootState), "Root state has no make_transition_table method");
        static_assert(
            bh::size(flatten_transition_table(rootState)),
            "Transition table needs at least one transition");
        static_assert(
            maxInitialStates(rootState),
            "Transition table needs to have at least one initial state");

        auto optionalDependency = bh::make_basic_tuple(std::ref(optionalParameters)...);

        if constexpr (has_unexpected_event_handler(rootState)) {
            fill_unexpected_event_handler_tables(
                rootState,
                m_statesMap,
                m_unexpectedEventHandlerTables,
                get_unexpected_event_handler(rootState),
                optionalDependency);
        }

        fill_dispatch_table(optionalDependency);
        fill_initial_state_table(rootState, m_initial_states);
        fill_initial_state_table(rootState, m_history);
        init_current_state();
        update_current_regions();
    }

    template <class Event> auto process_event(Event&& event) -> bool
    {
        static_assert(
            bh::contains(
                collect_event_typeids_recursive(rootState),
                bh::type_c<typename std::remove_reference<Event>::type>),
            "Processed event was not found in the transition table");

        switch (process_event_internal(event)) {
        case ProcessEventResult::AllTransitionsInvalid:
            call_unexpected_event_handler(event);
            [[fallthrough]];
        case ProcessEventResult::EventDefered:
        case ProcessEventResult::AllGuardsFailed:
            return false;
        case ProcessEventResult::EventProcessed:
            process_deferred_events();
        }
        return true;
    }

    template <class State> auto is(State state) -> bool
    {
        return currentState(0) == getStateIdx(rootState, state);
    }

    template <class ParentState, class State> auto is(ParentState parentState, State state) -> bool
    {
        return currentParentState() == getParentStateIdx(rootState, parentState)
            && currentState(0) == getStateIdx(rootState, state);
    }

    template <class ParentState, class State>
    auto is(Region region, ParentState parentState, State state) -> bool
    {
        return currentParentState() == getParentStateIdx(rootState, parentState)
            && currentState(region) == getStateIdx(rootState, state);
    }

    template <class ParentState> auto parent_is(ParentState parentState) -> bool
    {
        return currentParentState() == getParentStateIdx(rootState, parentState);
    }

    auto status() -> std::string
    {
        std::stringstream statusStream;
        const auto currentRegions = current_regions();
        for (Region region = 0; region < currentRegions; region++) {
            statusStream << "[" << region << "] "
                         << "combined: " << m_currentCombinedState[region] << " "
                         << "parent: " << currentParentState() << " "
                         << "state: " << currentState(region);
        }
        return statusStream.str();
    }

    auto set_dependency(OptionalParameters&... optionalParameters)
    {
        auto optionalDependency = bh::make_basic_tuple(std::ref(optionalParameters)...);
        fill_dispatch_table(optionalDependency);
    }

  private:
    enum class ProcessEventResult {
        AllTransitionsInvalid,
        AllGuardsFailed,
        EventProcessed,
        EventDefered
    };

    template <class Event> auto process_event_internal(Event&& event) -> ProcessEventResult
    {
        bool allGuardsFailed = true;
        bool allTransitionsInvalid = true;

        const auto currentRegions = current_regions();
        for (Region region = 0; region < currentRegions; region++) {

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            auto& results = get_dispatch_table_entry(event, region);

            if (results.empty()) {
                continue;
            }

            for (auto& result : results) {

                if (result.defer) {
                    m_defer_queue.push(event);
                    return ProcessEventResult::EventDefered;
                }

                if (!result.transition->executeGuard(event)) {
                    allTransitionsInvalid = false;
                    continue;
                }

                allTransitionsInvalid = false;
                allGuardsFailed = false;
                update_current_state(region, result);

                result.transition->executeAction(event);
                break;
            }
        }

        if (allTransitionsInvalid) {
            return ProcessEventResult::AllTransitionsInvalid;
        }

        if (allGuardsFailed) {
            return ProcessEventResult::AllGuardsFailed;
        }

        process_anonymous_transitions();
        return ProcessEventResult::EventProcessed;
    }

    auto process_deferred_events()
    {
        if constexpr (hasDeferedEvents(rootState)) {
            if (!m_defer_queue.empty()) {
                m_defer_queue.visit([this](auto event) { this->process_event_internal(event); });
            }
        }
    }

    auto process_anonymous_transitions()
    {
        if constexpr (has_anonymous_transition(rootState)) {
            while (true) {
                auto allGuardsFailed = true;

                const auto currentRegions = current_regions();
                for (Region region = 0; region < currentRegions; region++) {

                    auto event = noneEvent {};
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                    auto& results = get_dispatch_table_entry(event, region);

                    if (results.empty()) {
                        return;
                    }

                    for (auto& result : results) {
                        if (!result.transition->executeGuard(event)) {
                            continue;
                        }

                        update_current_state(region, result);
                        result.transition->executeAction(event);
                        allGuardsFailed = false;
                        break;
                    }
                }

                if (allGuardsFailed) {
                    return;
                }
            }
        }
    }

    template <class Event>
    constexpr auto dispatch_table_at(StateIdx index, const Event event) -> auto&
    {
        return m_dispatchTables[event][index];
    }

    template <class DispatchTableEntry>
    void update_current_state(Region region, const DispatchTableEntry& dispatchTableEntry)
    {
        if (dispatchTableEntry.internal) {
            return;
        }

        if constexpr (has_history(rootState)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            m_history[currentParentState()][region] = m_currentCombinedState[region];

            if (dispatchTableEntry.history) {
                auto parent
                    = calcParentStateIdx(nStates(rootState), dispatchTableEntry.combinedState);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                auto combined = m_history[parent][region];
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                m_currentCombinedState[region] = combined;
            } else {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                m_currentCombinedState[region] = dispatchTableEntry.combinedState;
            }
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            m_currentCombinedState[region] = dispatchTableEntry.combinedState;
        }

        update_current_regions();
    }

    void update_current_regions()
    {
        if constexpr (hasParallelRegions(rootState)) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            m_currentRegions = m_initial_states[currentParentState()].size();
        }
    }

    auto current_regions() -> std::size_t
    {
        if constexpr (hasParallelRegions(rootState)) {
            return m_currentRegions;
        } else {
            return 1;
        }
    }

    template <class Event> auto call_unexpected_event_handler(Event& event)
    {
        if constexpr (has_unexpected_event_handler(rootState)) {
            // TODO: What todo in a multi region state machine?
            m_unexpectedEventHandlerTables[bh::typeid_(event)]
                .at(m_currentCombinedState.at(0))
                ->executeHandler(event);
        }
    }

    auto currentState(Region region)
    {
        return calcStateIdx(nStates(rootState), m_currentCombinedState.at(region));
    }

    auto currentParentState()
    {
        return calcParentStateIdx(nStates(rootState), m_currentCombinedState[0]);
    }

    void init_current_state()
    {
        const auto initialParentState = getParentStateIdx(rootState, rootState);

        for (Region region = 0; region < m_initial_states[initialParentState].size(); region++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            m_currentCombinedState[region] = m_initial_states[initialParentState][region];
        }
    }

    template <class Event, class Region>
    constexpr auto get_dispatch_table_entry(Event& event, Region region) -> decltype(auto)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        return m_dispatchTables[bh::typeid_(event)][m_currentCombinedState[region]];
    }

    template <class OptionalDependency>
    void fill_dispatch_table(OptionalDependency& optionalDependency)
    {
        fill_dispatch_table_with_internal_transitions(
            rootState, m_dispatchTables, m_statesMap, optionalDependency);
        fill_dispatch_table_with_external_transitions(
            rootState, m_dispatchTables, m_statesMap, optionalDependency);
        fill_dispatch_table_with_deferred_events(rootState, m_dispatchTables, optionalDependency);
    }
};
}