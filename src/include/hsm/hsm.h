#pragma once

#include "details/collect_events.h"
#include "details/fill_dispatch_table.h"
#include "details/make_states_map.h"
#include "details/transition_table.h"
#include "details/variant_queue.h"

#include <boost/hana.hpp>

#include <array>
#include <cstdint>
#include <sstream>
#include <vector>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class RootState, class... OptionalParameters> class sm {
    using Region = std::uint8_t;
    using Events = decltype(collect_events_recursive(state<RootState> {}));
    using StatesMap = decltype(make_states_map(state<RootState> {}));
    std::array<StateIdx, maxInitialStates(state<RootState> {})> m_currentCombinedState;
    std::array<std::vector<std::size_t>, nParentStates(state<RootState> {})> m_initial_states;
    std::array<std::vector<std::size_t>, nParentStates(state<RootState> {})> m_history;
    variant_queue<Events> m_defer_queue;
    std::size_t m_currentRegions;
    StatesMap m_statesMap;

  public:
    sm(OptionalParameters... optionalParameters)
        : m_defer_queue(collect_events_recursive(state<RootState> {}))
        , m_statesMap(make_states_map(state<RootState> {}))
    {
        auto optionalDependency = bh::make_tuple(optionalParameters...);
        fill_dispatch_table_with_external_transitions(rootState(), m_statesMap, optionalDependency);
        fill_dispatch_table_with_internal_transitions(rootState(), m_statesMap, optionalDependency);
        fill_dispatch_table_with_deferred_events(rootState(), optionalDependency);
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
        // std::cout << "combined: " << m_currentCombinedState[0] << ", current: " <<
        // currentState(0)
        //           << " == " << getStateIdx(rootState(), state) << std::endl;

        return currentState(0) == getStateIdx(rootState(), state);
    }

    template <class ParentState, class State> auto is(ParentState parentState, State state) -> bool
    {
        // std::cout << "combined: " << m_currentCombinedState[0]
        //           << " parent: " << currentParentState()
        //           << " == " << getParentStateIdx(rootState(), parentState)
        //           << ", current: " << currentState(0) << " == " << getStateIdx(rootState(),
        //           state)
        //           << std::endl;

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
        // std::cout << "combined: " << m_currentCombinedState[0]
        //           << " parent: " << currentParentState()
        //           << " == " << getParentStateIdx(rootState(), parentState) << std::endl;

        return currentParentState() == getParentStateIdx(rootState(), parentState);
    }

    auto status() -> std::string
    {
        std::stringstream statusStream;
        for (std::size_t region = 0; region < current_regions(); region++) {
            statusStream << "[" << region << "] "
                         << "combined: " << m_currentCombinedState[region] << " "
                         << "parent: " << currentParentState() << " "
                         << "state: " << currentState(region);
        }
        return statusStream.str();
    }

  private:
    template <class Event> auto process_event_internal(Event&& event) -> bool
    {
        bool allGuardsFailed = true;

        for (std::size_t region = 0; region < current_regions(); region++) {

            auto& result = dispatch_table_at(m_currentCombinedState[region], event);

            if(result.defer){
                m_defer_queue.push(event);
                return true;
            }

            if(!result.valid){
                return false;    
            }

            if (!result.transition->executeGuard(event)) {
                continue;
            }

            allGuardsFailed = false;
            update_current_state(region, result);

            result.transition->executeAction(event);
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
                    m_defer_queue.visit([this](auto event) { process_event_internal(event); });
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

                    for (std::size_t region = 0; region < current_regions(); region++) {

                        auto event = noneEvent {};
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

    template <class Event> constexpr auto& dispatch_table_at(StateIdx index, const Event& /*event*/)
    {
        constexpr auto states = nStates(state<RootState> {}) * nParentStates(state<RootState> {});
        // std::cout << "access index: " << index << std::endl;
        return DispatchTable<states, Event>::table[index];
    }

    template <class DispatchTableEntry>
    void update_current_state(std::size_t region, const DispatchTableEntry& dispatchTableEntry)
    {
        bh::if_(
            has_history(rootState()),
            [&, this]() {
                m_history[currentParentState()][region] = m_currentCombinedState[region];

                if (dispatchTableEntry.history) {
                    m_currentCombinedState[region] = m_history[currentParentState()][region];
                } else {
                    m_currentCombinedState[region] = dispatchTableEntry.combinedState;
                }
            },
            [&, this]() { m_currentCombinedState[region] = dispatchTableEntry.combinedState; })();

        update_current_regions();
    }

    void update_current_regions()
    {
        bh::if_(
            hasRegions(rootState()),
            []() {},
            [this]() { m_currentRegions = m_initial_states[currentParentState()].size(); })();
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
        return state<RootState> {};
    }

    auto currentState(std::size_t region)
    {
        return calcStateIdx(nStates(rootState()), m_currentCombinedState[region]);
    }

    auto currentParentState()
    {
        return calcParentStateIdx(nStates(rootState()), m_currentCombinedState[0]);
    }

    void init_current_state()
    {
        auto initialParentState = getParentStateIdx(rootState(), rootState());

        for (std::size_t region = 0; region < m_initial_states[initialParentState].size();
             region++) {
            m_currentCombinedState[region] = calcCombinedStateIdx(
                nStates(rootState()),
                initialParentState,
                m_initial_states[initialParentState][region]);
        }
    }
};
}