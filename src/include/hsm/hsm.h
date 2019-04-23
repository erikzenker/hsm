#pragma once

#include "details/collect_events.h"
#include "details/collect_parent_states.h"
#include "details/collect_states.h"
#include "details/index_map.h"
#include "details/traits.h"

#include <boost/hana.hpp>

#include <cstdint>
#include <map>
#include <array>
#include <iostream>

namespace hsm {

    namespace bh{
        using namespace boost::hana; 
    };

    template <typename... Args> constexpr auto transition_table(Args... args)
    {
        return boost::hana::make_tuple(args...);
    }

    template <typename... Args> constexpr auto row(Args... args)
    {
        return boost::hana::make_tuple(args...);
    }

    template <typename... Args> constexpr auto transition(Args... args)
    {
        return boost::hana::make_tuple(args...);
    }

    template <class State>    
    class Sm
    {
        using Idx = std::uint16_t;
        using StateIdx = Idx;
        using EventIdx = Idx;
        using DispatchTable = std::array<
            std::map<StateIdx, std::map<EventIdx, std::pair<StateIdx, StateIdx>>>,
            bh::length(collect_parent_states(State {}))>;

        DispatchTable m_dispatchTable;
        StateIdx m_currentState;
        StateIdx m_currentParentState;

        public:
            Sm() : m_currentState(getStateIdx(inititalState())), m_currentParentState(getParentStateIdx(rootState()))
            {
                makeDispatchTable(rootState(), m_dispatchTable);
            }

            template <class T>
            auto process_event(T event)
            {
                std::tie(m_currentParentState, m_currentState) = m_dispatchTable[m_currentParentState].at(m_currentState).at(getEventIdx(event));
            }

            template <class T>
            auto is(T state) -> bool {
                return m_currentState == getStateIdx(state);
            };

            template <class T, class B>
            auto is(T parentState, B state) -> bool {
                return m_currentParentState == getParentStateIdx(parentState) && m_currentState == getStateIdx(state);
            };

        private:
            auto rootState(){
                return State{};    
            }

            auto inititalState(){
                return rootState().initial_state();    
            }

            auto parentStates(){
                return collect_parent_states(rootState());
            }

            constexpr auto states(){
                return collect_states_recursive(rootState());
            }

            auto events(){
                return collect_events_recursive(rootState());
            }

            template <class T, class B> auto makeDispatchTable(T state, B& dispatchTable)
            {
                auto fromParent = getParentStateIdx(state);

                bh::for_each(state.make_transition_table(), [&](auto row){
                    auto from = getStateIdx(bh::front(row));

                    auto to = bh::if_(has_transition_table(bh::back(row))
                        ,[this](auto state){ return getStateIdx(state.initial_state()); }
                        ,[this](auto state){ return getStateIdx(state);})(bh::back(row));

                    auto toParent = bh::if_(has_transition_table(bh::back(row))
                        ,[this](auto state){ return getParentStateIdx(state);}
                        ,[this, &state](auto){ return getParentStateIdx(state);})(bh::back(row));

                    auto with = getEventIdx(bh::at_c<1>(row));

                    m_dispatchTable[fromParent][from][with] = std::make_pair(toParent, to);

                    // Add dispatch table of sub states
                    bh::if_(
                        has_transition_table(bh::back(row)),
                        [this, &dispatchTable](auto state) {
                            makeDispatchTable(state, dispatchTable);
                        },
                        [](auto) {})(bh::back(row));

                    // Add dispatch table of sub states exits
                    bh::if_(
                        has_transition_table(bh::front(row)),
                        [this, with, toParent, to, &dispatchTable](auto parentState) {
                            auto states = collect_child_states(parentState);

                            bh::for_each(
                                states,
                                [this, parentState, with, toParent, to, &dispatchTable](
                                    auto state) {
                                    auto fromParent = getParentStateIdx(parentState);
                                    auto from = getStateIdx(state);
                                    m_dispatchTable[fromParent][from][with]
                                        = std::make_pair(toParent, to);
                                });

                            makeDispatchTable(parentState, dispatchTable);
                        },
                        [](auto) {})(bh::front(row));
                    
                });
            }

            template <class T>
            auto getStateIdx(T state){
                return getIdx(make_index_map(states()), bh::typeid_(state));
            }

            template <class T>
            auto getParentStateIdx(T parentState){
                return getIdx(make_index_map(parentStates()), bh::typeid_(parentState));
            }

            template <class T>
            auto getEventIdx(T event){
                return getIdx(make_index_map(events()), bh::typeid_(event));
            }

            template <class T, class B>
            auto getIdx(T map, B type) -> Idx {
                return bh::find(map, type).value();
            }
    };


}