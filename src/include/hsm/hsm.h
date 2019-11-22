#pragma once

#include "details/collect_actions.h"
#include "details/collect_events.h"
#include "details/collect_guards.h"
#include "details/collect_parent_states.h"
#include "details/collect_states.h"
#include "details/dispatch_table.h"
#include "details/event.h"
#include "details/flatten_internal_transition_table.h"
#include "details/flatten_transition_table.h"
#include "details/index_map.h"
#include "details/pseudo_states.h"
#include "details/switch.h"
#include "details/traits.h"
#include "details/transition_table.h"

#include <boost/hana.hpp>

#include <array>
#include <cstdint>
#include <iostream>
#include <map>

namespace hsm {

namespace bh {
using namespace boost::hana;
};

template <class RootState> class Sm {
    using Region = std::uint8_t;    
    std::array<StateIdx, maxInitialStates(RootState{})> m_currentState;
    StateIdx m_currentParentState;
    std::array<std::vector<std::size_t>, nParentStates(RootState{})> m_initial_states;
    std::array<std::vector<std::size_t>, nParentStates(RootState {})> m_history;

  public:
    Sm()
        : m_currentParentState(getParentStateIdx(rootState(), rootState()))
    {
        fill_dispatch_table_with_external_transitions(rootState());
        fill_dispatch_table_with_internal_transitions(rootState());
        fill_inital_state_table(rootState(), m_initial_states);
        fill_inital_state_table(rootState(), m_history);
        initCurrentState();
    }

    template <class Event> auto process_event(Event event)
    {
        try {
            process_event_internal(event);
        } catch (std::exception e) {
            call_unexpected_event_handler(event);
        }
    }

    template <class State> auto is(State state) -> bool
    {
        return m_currentState[0] == getStateIdx(rootState(), state);
    };

    template <class ParentState, class State> auto is(ParentState parentState, State state) -> bool
    {
        return m_currentParentState == getParentStateIdx(rootState(), parentState)
            && m_currentState[0] == getStateIdx(rootState(), state);
    };

    template <class ParentState, class State>
    auto is(Region region, ParentState parentState, State state) -> bool
    {
        return m_currentParentState == getParentStateIdx(rootState(), parentState)
            && m_currentState[region] == getStateIdx(rootState(), state);
    };

  private:
    template <class Event> auto process_event_internal(Event event)
    {
        bool allGuardsFailed = true;

        for (std::size_t region = 0; region < m_initial_states[m_currentParentState].size();
             region++) {

            auto& result = DispatchTable<RootState, Event>::table[m_currentParentState][m_currentState[region]];   

            if (!result.guard(event)) {
                continue;
            }

            allGuardsFailed = false;
            update_current_state(region, result);

            result.action(event);
        }

        if (allGuardsFailed) {
            return;
        }

        apply_anonymous_transitions();
    }

    auto apply_anonymous_transitions()
    {
        while (true) {

            for (std::size_t region = 0; region < m_initial_states[m_currentParentState].size();
                 region++) {

                auto event = noneEvent{};
                auto& result = DispatchTable<RootState, noneEvent>::table[m_currentParentState][m_currentState[region]];

                // Check if anonymous transition exists
                if (!result.guard) {
                    return;
                }

                if (!result.guard(event)) {
                    continue;
                }

                update_current_state(region, result);

                result.action(event);
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
        for(int region = 0; region < m_initial_states[m_currentParentState].size(); region++){
            m_currentState[region] = m_initial_states[m_currentParentState][region];
        }

    }    

};
}