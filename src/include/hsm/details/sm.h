#pragma once

#include "hsm/details/collect_events.h"
#include "hsm/details/fill_dispatch_table.h"
#include "hsm/details/idx.h"
#include "hsm/details/make_states_map.h"
#include "hsm/details/transition_table_traits.h"
#include "hsm/details/variant_queue.h"

#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/if.hpp>

#include <array>
#include <cstdint>
#include <sstream>
#include <vector>
#include <iostream>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class RootState, class... OptionalParameters> class sm {
    using Region = std::uint8_t;
    using Events = decltype(collect_event_typeids_recursive(state_t<RootState> {}));
    using StatesMap = decltype(make_states_map(state_t<RootState> {}));
    std::array<StateIdx, maxInitialStates(state_t<RootState> {})> m_currentCombinedState;
    std::array<std::vector<std::size_t>, nParentStates(state_t<RootState> {})> m_initial_states;
    std::array<std::vector<std::size_t>, nParentStates(state_t<RootState> {})> m_history;
    variant_queue<Events> m_defer_queue;
    std::size_t m_currentRegions{};
    StatesMap m_statesMap;

  public:
    sm(OptionalParameters&... optionalParameters)
        : m_initial_states()
        , m_history()
        , m_defer_queue(collect_event_typeids_recursive(state_t<RootState> {}))
        , m_statesMap(make_states_map(state_t<RootState> {}))
    {
        static_assert(
            has_transition_table(state_t<RootState> {}),
            "Root state has no make_transition_table method");
        static_assert(
            bh::size(flatten_transition_table(state_t<RootState> {})),
            "Transition table needs at least one transition");
        static_assert(
            maxInitialStates(state_t<RootState> {}),
            "Transition table needs to have at least one initial state");

        fill_dispatch_table(optionalParameters...);
        fill_initial_state_table(rootState(), m_initial_states);
        fill_initial_state_table(rootState(), m_history);
        init_current_state();
        update_current_regions();
    }

    template <class Event> auto process_event(Event&& event)
    {
        if (!process_event_internal(event)) {
            call_unexpected_event_handler(event);
            return;
        }

        process_deferred_events();
    }

    template <class State> auto is(State state) -> bool
    {
        return currentState(0) == getStateIdx(rootState(), state);
    }

    template <class ParentState, class State> auto is(ParentState parentState, State state) -> bool
    {
        return currentParentState() == getParentStateIdx(rootState(), parentState)
            && currentState(0) == getStateIdx(rootState(), state);
    }

    template <class ParentState, class State>
    auto is(Region region, ParentState parentState, State state) -> bool
    {
        return currentParentState() == getParentStateIdx(rootState(), parentState)
            && currentState(region) == getStateIdx(rootState(), state);
    }

    template <class ParentState> auto parent_is(ParentState parentState) -> bool
    {
        return currentParentState() == getParentStateIdx(rootState(), parentState);
    }

    auto status() -> std::string
    {
        std::stringstream statusStream;
        for (Region region = 0; region < current_regions(); region++) {
            statusStream << "[" << region << "] "
                         << "combined: " << m_currentCombinedState[region] << " "
                         << "parent: " << currentParentState() << " "
                         << "state: " << currentState(region);
        }
        return statusStream.str();
    }

    auto set_dependency(OptionalParameters&... optionalParameters)
    {
        fill_dispatch_table(optionalParameters...);
    }

  private:
    template <class Event> auto process_event_internal(Event&& event) -> bool
    {
        bool allGuardsFailed = true;
        bool allTransitionsInvalid = true;

        for (Region region = 0; region < current_regions(); region++) {

            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            auto& result = dispatch_table_at(m_currentCombinedState[region], event);

            if(result.defer){
                m_defer_queue.push(event);
                return true;
            }

            if(!result.valid){
                continue;
            }

            if (!result.transition->executeGuard(event)) {
                continue;
            }

            allTransitionsInvalid = false;
            allGuardsFailed = false;
            update_current_state(region, result);

            result.transition->executeAction(event);
        }

        if (allTransitionsInvalid) {
            return false;
        }

        if (allGuardsFailed) {
            return true;
        }

        apply_anonymous_transitions();
        return true;
    }

    auto process_deferred_events()
    {
        bh::if_(
            hasDeferedEvents(rootState()),
            [this]() {
                if (!m_defer_queue.empty()) {
                    m_defer_queue.visit([this](auto event) { this->process_event_internal(event); });
                }
            },
            []() {})();
    }

    auto apply_anonymous_transitions()
    {
        bh::if_(
            has_anonymous_transition(rootState()),
            [this]() {
                while (true) {
                    for (Region region = 0; region < current_regions(); region++) {

                        auto event = noneEvent {};
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                        auto& result = dispatch_table_at(m_currentCombinedState[region], event);

                        if (!result.valid) {
                            return;
                        }

                        if (!result.transition->executeGuard(event)) {
                            continue;
                        }

                        update_current_state(region, result);
                        result.transition->executeAction(event);
                    }
                }
            },
            []() {})();
    }

    template <class Event> constexpr auto dispatch_table_at(StateIdx index, const Event& /*event*/) -> auto&
    {
        return bh::apply(
            [](auto states, StateIdx index) -> auto& {
                return DispatchTable<states, Event>::table[index];
            },
            nStates(state_t<RootState> {}) * nParentStates(state_t<RootState> {}),
            index);
    }

    template <class DispatchTableEntry>
    void update_current_state(Region region, const DispatchTableEntry& dispatchTableEntry)
    {
        bh::if_(
            has_history(rootState()),
            [&, this]() {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)    
                m_history[currentParentState()][region] = m_currentCombinedState[region];

                if (dispatchTableEntry.history) {
                    auto parent = calcParentStateIdx(
                        nStates(rootState()), dispatchTableEntry.combinedState);
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                    auto combined = m_history[parent][region];
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                    m_currentCombinedState[region] = combined;
                } else {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)    
                    m_currentCombinedState[region] = dispatchTableEntry.combinedState;
                }
            },
            [&, this]() { 
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)        
                m_currentCombinedState[region] = dispatchTableEntry.combinedState; 
            })();

        update_current_regions();
    }

    void update_current_regions()
    {
        bh::if_(
            hasRegions(rootState()),
            []() {},
            [this]() { 
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)        
                m_currentRegions = m_initial_states[currentParentState()].size(); 
            })();
    }

    auto current_regions() -> std::size_t
    {
        return bh::if_(
            hasRegions(rootState()), [](auto) { return 1; }, [](auto regions) { return regions; })(
            m_currentRegions);
    }

    template <class Event> auto call_unexpected_event_handler(Event event)
    {
        const auto handler = get_unexpected_event_handler(rootState());
        handler(event);
    }

    constexpr auto rootState()
    {
        return state_t<RootState> {};
    }

    auto currentState(Region region)
    {
        return calcStateIdx(nStates(rootState()), m_currentCombinedState.at(region));
    }

    auto currentParentState()
    {
        return calcParentStateIdx(nStates(rootState()), m_currentCombinedState[0]);
    }

    void init_current_state()
    {
        const auto initialParentState = getParentStateIdx(rootState(), rootState());

        for (Region region = 0; region < m_initial_states[initialParentState].size();
             region++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            m_currentCombinedState[region] = m_initial_states[initialParentState][region];
        }
    }

    void fill_dispatch_table(OptionalParameters&... optionalParameters)
    {
        auto optionalDependency = bh::make_basic_tuple(std::ref(optionalParameters)...);
        fill_dispatch_table_with_internal_transitions(rootState(), m_statesMap, optionalDependency);
        fill_dispatch_table_with_external_transitions(rootState(), m_statesMap, optionalDependency);
        fill_dispatch_table_with_deferred_events(rootState(), optionalDependency);
    }
};
}