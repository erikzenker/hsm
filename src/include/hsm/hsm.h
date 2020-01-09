#pragma once

#include "details/collect_actions.h"
#include "details/collect_events.h"
#include "details/collect_guards.h"
#include "details/collect_parent_states.h"
#include "details/collect_states.h"
#include "details/dispatch_table.h"
#include "details/flatten_internal_transition_table.h"
#include "details/flatten_transition_table.h"
#include "details/index_map.h"
#include "details/pseudo_states.h"
#include "details/switch.h"
#include "details/traits.h"
#include "details/transition_table.h"
#include "details/variant_queue.h"

#include "front/transition.h"

#include <boost/hana.hpp>

#include <array>
#include <cstdint>
#include <iostream>
#include <map>

namespace hsm {

namespace bh {
using namespace boost::hana;
}

template <class RootState, class... OptionalParameters> class Sm {
    using Region = std::uint8_t;
    using Events = decltype(collect_events_recursive(RootState {}));
    std::array<StateIdx, maxInitialStates(RootState{})> m_currentState;
    StateIdx m_currentParentState;
    std::array<std::vector<std::size_t>, nParentStates(RootState{})> m_initial_states;
    std::array<std::vector<std::size_t>, nParentStates(RootState {})> m_history;
    bh::tuple<OptionalParameters...> m_optionalParameters;
    variant_queue<Events> m_defer_queue;

  public:
    Sm(OptionalParameters... optionalParameters)
        : m_currentParentState(getParentStateIdx(rootState(), rootState()))
        , m_defer_queue(collect_events_recursive(RootState {}))
    {
        m_optionalParameters = bh::make_tuple(optionalParameters...);
        fill_dispatch_table_with_external_transitions(rootState(), optionalParameters...);
        fill_dispatch_table_with_internal_transitions(rootState(), optionalParameters...);
        fill_dispatch_table_with_deferred_events(rootState(), optionalParameters...);
        fill_inital_state_table(rootState(), m_initial_states);
        fill_inital_state_table(rootState(), m_history);
        initCurrentState();
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
        return m_currentState[0] == getStateIdx(rootState(), state);
    }

    template <class ParentState, class State> auto is(ParentState parentState, State state) -> bool
    {
        return m_currentParentState == getParentStateIdx(rootState(), parentState)
            && m_currentState[0] == getStateIdx(rootState(), state);
    }

    template <class ParentState, class State>
    auto is(Region region, ParentState parentState, State state) -> bool
    {
        return m_currentParentState == getParentStateIdx(rootState(), parentState)
            && m_currentState[region] == getStateIdx(rootState(), state);
    }

  private:
    template <class Event> auto process_event_internal(Event event) -> bool
    {
        bool allGuardsFailed = true;

        for (std::size_t region = 0; region < m_initial_states[m_currentParentState].size();
             region++) {

            auto& result = DispatchTable<RootState, Event, OptionalParameters...>::table
                [m_currentParentState][m_currentState[region]];

            if(result.defer){
                m_defer_queue.push(event);
                return true;
            }

            if (!result.guard) {
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
        if (!m_defer_queue.empty()) {
            m_defer_queue.visit([this](auto event) { process_event_internal(event); });
        }
    }

    auto apply_anonymous_transitions()
    {
        while (true) {

            for (std::size_t region = 0; region < m_initial_states[m_currentParentState].size();
                 region++) {

                auto event = noneEvent{};
                auto& result = DispatchTable<RootState, noneEvent, OptionalParameters...>::table
                    [m_currentParentState][m_currentState[region]];

                // Check if anonymous transition exists
                if (!result.guard) {
                    return;
                }

                if (!call_guard(result.guard, event)) {
                    continue;
                }

                update_current_state(region, result);
                call_action(result.action, event);
            }
        }
    }

    template <class DispatchTableEntry>
    void update_current_state(std::size_t region, const DispatchTableEntry& dispatchTableEntry)
    {
        m_history[m_currentParentState][region] = m_currentState[region];
        m_currentParentState = dispatchTableEntry.parentState;

        if (dispatchTableEntry.history) {
            m_currentState[region] = m_history[m_currentParentState][region];
        } else {
            m_currentState[region] = dispatchTableEntry.state;
        }
    }

    template <class Action, class Event> auto call_action(const Action& action, const Event& event)
    {
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

    void initCurrentState()
    {
        for(std::size_t region = 0; region < m_initial_states[m_currentParentState].size(); region++){
            m_currentState[region] = m_initial_states[m_currentParentState][region];
        }

    }
};
}