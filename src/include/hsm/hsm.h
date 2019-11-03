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
    std::array<StateIdx, maxRegions(RootState{})> m_currentState;
    StateIdx m_currentParentState;
    std::array<std::vector<std::size_t>, nParentStates(RootState{})> m_regions;

  public:
    Sm()
        : m_currentParentState(getParentStateIdx(rootState(), rootState()))
    {
        m_currentState[0] = getStateIdx(rootState(), initialState());
        fill_dispatch_table(rootState());
        fill_dispatch_table2(rootState());
        make_region_map(rootState(), m_regions);

        for(int region = 0; region < m_regions[m_currentParentState].size(); region++){
            m_currentState[region] = m_regions[m_currentParentState][region];
        }
    }

    template <class Event> auto process_event(Event event)
    {
        for(int region = 0; region < m_regions[m_currentParentState].size(); region++){

            auto& result = DispatchTable<RootState, Event>::table[m_currentParentState][m_currentState[region]];   

            if (!result.guard(event)) {
                // TODO: just return if all guards fail    
                return;
            }

            m_currentParentState = result.parentState;
            m_currentState[region] = result.state;

            result.action(event);
        }


        apply_anonymous_transitions();
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

    template <class ParentState, class State> auto is(Region region, ParentState parentState, State state) -> bool
    {
        return m_currentParentState == getParentStateIdx(rootState(), parentState)
            && m_currentState[region] == getStateIdx(rootState(), state);
        
    };

  private:
    auto apply_anonymous_transitions()
    {
        while (true) {

            for(int region = 0; region < m_regions[m_currentParentState].size(); region++){

                auto event = noneEvent{};
                auto& result = DispatchTable<RootState, noneEvent>::table[m_currentParentState][m_currentState[region]];

                if (!result.guard) {
                    return;
                }

                if (!result.guard(event)) {
                    // TODO: just return if all guards fail        
                    return;
                }

                m_currentParentState = result.parentState;
                m_currentState[region] = result.state;

                result.action(event);
            }
        }
    }

    auto rootState()
    {
        return RootState {};
    }

    auto initialState()
    {
        return bh::at_c<0>(rootState().initial_state()); // TODO: make multi region capable
    }

    // auto initialState(Region region)
    // {
    //     return bh::at_c<0>(rootState().initial_state()); // TODO: make multi region capable
    // }
};
}