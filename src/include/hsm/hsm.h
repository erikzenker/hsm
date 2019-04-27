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

    struct none {
    };

    template <class ParentState, class State> class Exit {
      public:
        constexpr Exit(ParentState parentState, State state)
            : parentState(parentState)
            , state(state)
        {
        }

        constexpr auto get_parent_state()
        {
            return parentState;
        }

        constexpr auto get_state()
        {
            return state;
        }

      private:
        ParentState parentState;
        State state;
    };

    template <class State>    
    class Sm
    {
        using Idx = std::uint16_t;
        using StateIdx = Idx;
        using EventIdx = Idx;
        using DispatchTable = std::array<
            std::map<StateIdx, std::map<EventIdx, std::pair<StateIdx, StateIdx>>>,
            bh::length(collect_parent_states(State {}))>;
        using AnonymousDispatchTable = std::array<
            std::map<StateIdx, std::pair<StateIdx, StateIdx>>,
            bh::length(collect_parent_states(State {}))>;

        DispatchTable m_dispatchTable;
        AnonymousDispatchTable m_anonymousDispatchTable;
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
                apply_anonymous_transitions();
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
          auto apply_anonymous_transitions()
          {
              while (true) {
                  auto it = m_anonymousDispatchTable[m_currentParentState].find(m_currentState);
                  if (it == m_anonymousDispatchTable[m_currentParentState].end()) {
                      break;
                  }

                  std::tie(m_currentParentState, m_currentState)
                      = m_anonymousDispatchTable[m_currentParentState].at(m_currentState);
              }
          }

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
                bh::if_(
                    has_transition_table(state),
                    [this, &dispatchTable](auto state, auto& dispatchTable) {
                        makeDispatchTable2(state, dispatchTable);
                    },
                    [](auto, auto&) {})(state, dispatchTable);
            }

            template <class T, class B> auto makeDispatchTable2(T state, B& dispatchTable)
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

                    const auto is_anonymous_transition = [](auto transition) {
                        return bh::typeid_(bh::at_c<1>(transition)) == bh::typeid_(none {});
                    };

                    // Add entries for anonymous transition table

                    // Add dispatch table entries
                    bh::if_(
                        is_exit_state(bh::front(row)),
                        [&](auto exit) {
                            fromParent = getParentStateIdx(exit.get_parent_state());
                            from = getStateIdx(exit.get_state());

                            if (is_anonymous_transition(row)) {
                                //  ...anonymous pseudo exits
                                m_anonymousDispatchTable[fromParent][from]
                                    = std::make_pair(toParent, to);
                            } else {
                                // ...not anonymous pseudo exits
                                dispatchTable[fromParent][from][with]
                                    = std::make_pair(toParent, to);
                            }
                        },
                        [&](auto) {
                            if (is_anonymous_transition(row)) {
                                //  ...anonymous transitions
                                m_anonymousDispatchTable[fromParent][from]
                                    = std::make_pair(toParent, to);
                            } else {
                                //  ...not anonymous transitions
                                dispatchTable[fromParent][from][with]
                                    = std::make_pair(toParent, to);
                            }
                        })(bh::front(row));

                    makeDispatchTable(bh::back(row), dispatchTable);

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
                                    dispatchTable[fromParent][from][with]
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