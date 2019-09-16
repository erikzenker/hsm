#pragma once

#include "details/collect_actions.h"
#include "details/collect_events.h"
#include "details/collect_guards.h"
#include "details/collect_parent_states.h"
#include "details/collect_states.h"
#include "details/dispatch_table.h"
#include "details/event.h"
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
    StateIdx m_currentState;
    StateIdx m_currentParentState;
  public:
    Sm()
        : m_currentState(getStateIdx(rootState(), inititalState()))
        , m_currentParentState(getParentStateIdx(rootState(), rootState()))
    {
        fill_dispatch_table(rootState());
    }

    template <class Event> auto process_event(Event event)
    {
        auto& result = DispatchTable<RootState, Event>::table[m_currentParentState][m_currentState];   

        if (!result.guard(event)) {
            return;
        }

        m_currentParentState = result.parentState;
        m_currentState = result.state;

        result.action(event);
        apply_anonymous_transitions();
    }

    template <class State> auto is(State state) -> bool
    {
        return m_currentState == getStateIdx(rootState(), state);
    };

    template <class ParentState, class State> auto is(ParentState parentState, State state) -> bool
    {
        return m_currentParentState == getParentStateIdx(rootState(), parentState)
            && m_currentState == getStateIdx(rootState(), state);
    };

  private:
    auto apply_anonymous_transitions()
    {
        while (true) {
            auto event = noneEvent{};
            auto& result = DispatchTable<RootState, noneEvent>::table[m_currentParentState][m_currentState];

            if (!result.guard) {
                break;
            }

            if (!result.guard(event)) {
                return;
            }

            m_currentParentState = result.parentState;
            m_currentState = result.state;

            result.action(event);
        }
    }

    auto rootState()
    {
        return RootState {};
    }

    auto inititalState()
    {
        return rootState().initial_state();
    }

};
}