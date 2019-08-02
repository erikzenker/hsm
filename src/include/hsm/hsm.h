#pragma once

#include "details/call.h"
#include "details/collect_actions.h"
#include "details/collect_events.h"
#include "details/collect_guards.h"
#include "details/collect_parent_states.h"
#include "details/collect_states.h"
#include "details/event.h"
#include "details/index_map.h"
#include "details/pseudo_states.h"
#include "details/traits.h"
#include "details/transition_table.h"

#include <boost/hana.hpp>

#include <cstdint>
#include <map>
#include <array>
#include <iostream>

namespace hsm {

    namespace bh{
        using namespace boost::hana; 
    };

    template <class RootState> class Sm {
        using Idx = std::uint16_t;
        using StateIdx = Idx;
        using EventIdx = Idx;
        using ActionIdx = Idx;
        using GuardIdx = Idx;
        using DispatchTable = std::array<
            std::map<
                StateIdx,
                std::map<EventIdx, std::tuple<StateIdx, StateIdx, GuardIdx, ActionIdx>>>,
            bh::length(collect_parent_states(RootState {}))>;
        using AnonymousDispatchTable = std::array<
            std::map<StateIdx, std::tuple<StateIdx, StateIdx>>,
            bh::length(collect_parent_states(RootState {}))>;

        DispatchTable m_dispatchTable;
        AnonymousDispatchTable m_anonymousDispatchTable;
        StateIdx m_currentState;
        StateIdx m_currentParentState;

        public:
            Sm() : m_currentState(getStateIdx(inititalState())), m_currentParentState(getParentStateIdx(rootState()))
            {
                makeDispatchTable(rootState(), m_dispatchTable);
            }

            template <class Event>
            auto process_event(Event event)
            {
                const auto [nextParentState, nextState, guardIdx, actionIdx]
                    = m_dispatchTable[m_currentParentState]
                          .at(m_currentState)
                          .at(getEventIdx(event));

                if (!call_guard(guardIdx, guards(), event)) {
                    return;
                }

                m_currentParentState = nextParentState;
                m_currentState = nextState;
                call(actionIdx, actions(), event);
                apply_anonymous_transitions();
            }

            template <class State> auto is(State state) -> bool
            {
                return m_currentState == getStateIdx(state);
            };

            template <class ParentState, class State>
            auto is(ParentState parentState, State state) -> bool
            {
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
                return RootState {};
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

            auto actionTypeids()
            {
                return collect_action_typeids_recursive(rootState());
            }

            auto guardTypeids()
            {
                return collect_guard_typeids_recursive(rootState());
            }

            auto actions()
            {
                return collect_actions_recursive(rootState());
            }

            auto guards()
            {
                return collect_guards_recursive(rootState());
            }

            template <class Transition>
            auto fromStateIdx(Transition transition){
                    
            }

            template <class ParentState, class Transition, class DispatchTable>
            auto addDispatchTableEntry(ParentState parentState, Transition row, DispatchTable& dispatchTable){
                    auto fromParent = getParentStateIdx(parentState);
                    auto from = getStateIdx(bh::front(row));

                    auto to = bh::if_(has_transition_table(bh::back(row))
                        ,[this](auto submachine){ return getStateIdx(submachine.initial_state()); }
                        ,[this](auto state){ return getStateIdx(state);})(bh::back(row));

                    auto toParent = bh::if_(has_transition_table(bh::back(row))
                        ,[this](auto submachine){ return getParentStateIdx(submachine);}
                        ,[this, &parentState](auto){ return getParentStateIdx(parentState);})(bh::back(row));

                    auto with = getEventIdx(bh::at_c<1>(row));

                    auto action = getActionIdx(bh::at_c<3>(row));
                    auto guard = getGuardIdx(bh::at_c<2>(row));

                    const auto is_anonymous_transition = [](auto transition) {
                        return bh::typeid_(bh::at_c<1>(transition)) == bh::typeid_(none {});
                    };

                    bh::if_(
                        is_exit_state(bh::front(row)),
                        [&](auto exit) {
                            fromParent = getParentStateIdx(exit.get_parent_state());
                            from = getStateIdx(exit.get_state());

                            if (is_anonymous_transition(row)) {
                                //  ...anonymous pseudo exits
                                m_anonymousDispatchTable[fromParent][from]
                                    = std::make_tuple(toParent, to);
                            } else {
                                // ...not anonymous pseudo exits
                                dispatchTable[fromParent][from][with]
                                    = std::make_tuple(toParent, to, guard, action);
                            }
                        },
                        [&](auto) {
                            if (is_anonymous_transition(row)) {
                                //  ...anonymous transitions
                                m_anonymousDispatchTable[fromParent][from]
                                    = std::make_tuple(toParent, to);
                            } else {
                                //  ...not anonymous transitions
                                dispatchTable[fromParent][from][with]
                                    = std::make_tuple(toParent, to, guard, action);
                            }
                        })(bh::front(row));

            }

            template <class ParentState, class Transition, class DispatchTable>
            auto addDispatchTableEntryOfSubMachineExits(ParentState state, Transition row, DispatchTable& dispatchTable){
                    auto fromParent = getParentStateIdx(state);
                    auto from = getStateIdx(bh::front(row));

                    auto to = bh::if_(has_transition_table(bh::back(row))
                        ,[this](auto submachine){ return getStateIdx(submachine.initial_state()); }
                        ,[this](auto state){ return getStateIdx(state);})(bh::back(row));

                    auto toParent = bh::if_(has_transition_table(bh::back(row))
                        ,[this](auto submachine){ return getParentStateIdx(submachine);}
                        ,[this, &state](auto){ return getParentStateIdx(state);})(bh::back(row));

                    auto with = getEventIdx(bh::at_c<1>(row));

                    auto action = getActionIdx(bh::at_c<3>(row));
                    auto guard = getGuardIdx(bh::at_c<2>(row));

                    const auto is_anonymous_transition = [](auto transition) {
                        return bh::typeid_(bh::at_c<1>(transition)) == bh::typeid_(none {});
                    };


                    bh::if_(
                        has_transition_table(bh::front(row)),
                        [this, with, toParent, to, guard, action, &dispatchTable](
                            auto parentState) {
                            auto states = collect_child_states(parentState);

                            bh::for_each(
                                states,
                                [this,
                                 parentState,
                                 with,
                                 toParent,
                                 to,
                                 guard,
                                 action,
                                 &dispatchTable](auto state) {
                                    auto fromParent = getParentStateIdx(parentState);
                                    auto from = getStateIdx(state);
                                    dispatchTable[fromParent][from][with]
                                        = std::make_tuple(toParent, to, guard, action);
                                });

                            makeDispatchTable(parentState, dispatchTable);
                        },
                        [](auto) {})(bh::front(row));

            }

            template <class State, class DispatchTable>
            auto makeDispatchTable(State state, DispatchTable& dispatchTable)
            {
                bh::if_(
                    has_transition_table(state),
                    [this](auto state, auto& dispatchTable) {
                        makeDispatchTable2(state, dispatchTable);
                    },
                    [](auto, auto&) {})(state, dispatchTable);
            }

            template <class State, class DispatchTable>
            auto makeDispatchTable2(State state, DispatchTable& dispatchTable)
            {
                bh::for_each(state.make_transition_table(), [&](auto row){
                    addDispatchTableEntry(state, row, dispatchTable);
                    makeDispatchTable(bh::back(row), dispatchTable);
                    addDispatchTableEntryOfSubMachineExits(state, row, dispatchTable);
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
                auto then = [](auto event){return bh::typeid_(event.getEvent());};
                auto otherwise = [](auto event){return bh::typeid_(event);};
                auto eventId = bh::if_(is_event(event), then, otherwise)(event);

                return getIdx(make_index_map(events()), eventId);
            }

            template <class T> auto getActionIdx(T action)
            {
                return getIdx(make_index_map(actionTypeids()), bh::typeid_(action));
            }

            template <class T> auto getGuardIdx(T guard)
            {
                return getIdx(make_index_map(guardTypeids()), bh::typeid_(guard));
            }

            template <class T, class B>
            auto getIdx(T map, B type) -> Idx {
                return bh::find(map, type).value();
            }
    };
}