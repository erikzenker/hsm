#pragma once

#include "details/collect_events.h"
#include "details/dispatch_table.h"
#include "details/transition_table.h"
#include "details/variant_queue.h"

#include <boost/hana.hpp>

#include <array>
#include <vector>
#include <cstdint>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class RootState, class... OptionalParameters> class sm {
    using Region = std::uint8_t;
    using Events = decltype(collect_events_recursive(RootState {}));
    std::array<StateIdx, maxInitialStates(RootState{})> m_currentCombinedState;
    std::array<std::vector<std::size_t>, nParentStates(RootState{})> m_initial_states;
    std::array<std::vector<std::size_t>, nParentStates(RootState {})> m_history;
    bh::tuple<OptionalParameters...> m_optionalParameters;
    variant_queue<Events> m_defer_queue;
    std::size_t m_currentRegions;

  public:
    sm(OptionalParameters... optionalParameters)
        : m_defer_queue(collect_events_recursive(RootState {}))
    {
        m_optionalParameters = bh::make_tuple(optionalParameters...);
        fill_dispatch_table_with_external_transitions(rootState(), optionalParameters...);
        fill_dispatch_table_with_internal_transitions(rootState(), optionalParameters...);
        fill_dispatch_table_with_deferred_events(rootState(), optionalParameters...);
        fill_inital_state_table(rootState(), m_initial_states);
        fill_inital_state_table(rootState(), m_history);
        init_current_state();
        update_current_regions();
    }

    template <class Event> auto process_event(Event event)
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

  private:
    template <class Event> auto process_event_internal(Event event) -> bool
    {
        bool allGuardsFailed = true;

        for (std::size_t region = 0; region < current_regions(); region++) {

            auto& result = DispatchTable<RootState, Event, OptionalParameters...>::table
                [m_currentCombinedState[region]];

            if(result.defer){
                m_defer_queue.push(event);
                return true;
            }

            if(!result.valid){
                return false;    
            }

            if (!call_guard(result.guard, event)) {
                continue;
            }

            allGuardsFailed = false;
            update_current_state(region, result);
            call_action(result.action, event);
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
                        auto& result = DispatchTable<RootState, noneEvent, OptionalParameters...>::
                            table[m_currentCombinedState[region]];

                        if (!result.valid) {
                            return;
                        }

                        if (!call_guard(result.guard, event)) {
                            continue;
                        }

                        update_current_state(region, result);
                        call_action(result.action, event);
                    }
                }
            },
            []() {})();
    }

    template <class DispatchTableEntry>
    void update_current_state(std::size_t region, const DispatchTableEntry& dispatchTableEntry)
    {
        bh::if_(has_history(rootState()),
            [&, this](){
                m_history[currentParentState()][region] = m_currentCombinedState[region];

                if (dispatchTableEntry.history) {
                    m_currentCombinedState[region] = m_history[currentParentState()][region];
                } else {
                    m_currentCombinedState[region] = dispatchTableEntry.combinedState;
                }
            },
            [&, this](){
                m_currentCombinedState[region] = dispatchTableEntry.combinedState;
            }
        )();

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

    template <class Action, class Event> auto call_action(const Action& action, const Event& event)
    {
        if(!action){
            return;    
        }

        bh::if_(
            contains_dependency(m_optionalParameters),
            [](const auto& action, const auto& event, const auto& parameters) {
                action(event, bh::at_c<0>(parameters));
            },
            [](const auto& action, const auto& event, const auto&) { action(event); })(
            action, event, m_optionalParameters);
    }

    template <class Guard, class Event>
    auto call_guard(const Guard& guard, const Event& event) -> bool
    {
        if(!guard){
            return true;
        }

        return bh::if_(
            contains_dependency(m_optionalParameters),
            [](const auto& guard, const auto& event, const auto& parameters) {
                return guard(event, bh::at_c<0>(parameters));
            },
            [](const auto& guard, const auto& event, const auto&) { return guard(event); })(
            guard, event, m_optionalParameters);
    }

    template <class Event> auto call_unexpected_event_handler(Event event)
    {
        const auto handler = get_unexpected_event_handler(rootState());
        handler(event);
    }

    auto rootState()
    {
        return RootState {};
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
                nStates(RootState {}),
                initialParentState,
                m_initial_states[initialParentState][region]);
        }
    }
};
}