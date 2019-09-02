#pragma once

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
#include "details/action_map.h"
#include "details/flatten_transition_table.h"
#include "details/dispatch_table.h"
#include "details/switch.h"

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
        decltype(make_action_map(RootState {})) m_actionMap;
        decltype(make_guard_map(RootState {})) m_guardMap;
        StateIdx m_currentState;
        StateIdx m_currentParentState;

        public:
            Sm() : m_currentState(getStateIdx(inititalState())), m_currentParentState(getParentStateIdx(rootState()))
            {
                makeDispatchTable(rootState(), m_dispatchTable);
                makeActionMap(rootState(), m_actionMap);
                makeGuardMap(rootState(), m_guardMap);
            }

            template <class Event>
            auto process_event(Event event)
            {
                const auto [nextParentState, nextState, guardIdx, actionIdx]
                    = m_dispatchTable[m_currentParentState]
                          .at(m_currentState)
                          .at(getEventIdx(event));

                if (!callGuard(guardIdx, event)) {
                    return;
                }

                m_currentParentState = nextParentState;
                m_currentState = nextState;
                
                callAction(actionIdx, event);
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
                return collect_events_typeids_recursive(rootState());
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

            template <class Event>             
            auto callAction(ActionIdx actionIdx, Event event)
            {
                boost::hana::find(m_actionMap, boost::hana::typeid_(event)).value()[actionIdx](event);
            }

            template <class Event> bool callGuard(GuardIdx guardIdx, Event event)
            {
                return boost::hana::find(m_guardMap, boost::hana::typeid_(event))
                    .value()[guardIdx](event);
            }

            template <class State, class ActionMap>
            auto makeActionMap(State state, ActionMap& actionMap){
                actionMap = make_action_map(state);
            }

            template <class State, class GuardMap>
            auto makeGuardMap(State state, GuardMap& guardMap)
            {
                guardMap = make_guard_map(state);
            }

            template <class Transition>
            auto resolveDst(Transition transition){
                    return switch_(
                            case_(has_transition_table, [this](auto submachine){ return submachine.initial_state(); }),
                            case_(is_entry_state, [](auto entry){ return entry.get_state();}),
                            case_(otherwise, [this](auto state){ return state;})
                    )(getDst(transition));
            }


            template <class Transition>
            auto resolveDstParent(Transition transition){
                    return switch_(
                            case_(has_transition_table, [this](auto submachine){ return submachine;}),
                            case_(is_entry_state, [](auto entry){ return entry.get_parent_state();}),
                            case_(otherwise, [this, &transition](auto){ return getSrcParent(transition);})
                    )(getDst(transition));
            }

            template <class Transition>
            auto resolveSrc(Transition transition){
                    return switch_(
                            case_(is_exit_state, [](auto exit){ return exit.get_state();}),    
                            case_(otherwise, [](auto state){ return state;})    
                    )(getSrc(transition));
            }

            template <class Transition>
            auto resolveSrcParent(Transition transition){
                    return switch_(
                            case_(is_exit_state, [](auto exit){ return exit.get_parent_state();}),
                            case_(otherwise, [transition](auto){ return getSrcParent(transition);})
                    )(getSrc(transition));
            }

            template <class Transition, class DispatchTable>
            auto addDispatchTableEntry(Transition transition, DispatchTable& dispatchTable){
                    auto fromParent = getParentStateIdx(resolveSrcParent(transition));
                    auto from = getStateIdx(resolveSrc(transition));
                    auto with = getEventIdx(getEvent(transition));
                    auto guard = getGuardIdx(getGuard(transition));
                    auto action = getActionIdx(getAction(transition));
                    auto toParent = getParentStateIdx(resolveDstParent(transition));
                    auto to = getStateIdx(resolveDst(transition));

                    if (is_anonymous_transition(transition)) {
                        m_anonymousDispatchTable[fromParent][from] = std::make_tuple(toParent, to);
                    } else {
                        dispatchTable[fromParent][from][with]
                            = std::make_tuple(toParent, to, guard, action);
                    }
            }

            template <class Transition, class DispatchTable>
            auto addDispatchTableEntryOfSubMachineExits(Transition transition, DispatchTable& dispatchTable){
                    bh::if_(
                        has_transition_table(getSrc(transition)),
                        [this, &dispatchTable, transition](
                            auto parentState) {
                            auto states = collect_child_states(parentState);

                            bh::for_each(
                                states,
                                [this,
                                 parentState,
                                 &dispatchTable,
                                 transition](auto state) {
                                    auto fromParent = getParentStateIdx(parentState);
                                    auto from = getStateIdx(state);
                                    auto with = getEventIdx(getEvent(transition));
                                    auto guard = getGuardIdx(getGuard(transition));
                                    auto action = getActionIdx(getAction(transition));
                                    auto toParent = getParentStateIdx(resolveDstParent(transition));
                                    auto to = getStateIdx(resolveDst(transition));

                                    dispatchTable[fromParent][from][with]
                                        = std::make_tuple(toParent, to, guard, action);
                                });

                        },
                        [](auto) {})(getSrc(transition));

            }

            template <class ParentState, class DispatchTable>
            auto makeDispatchTable(ParentState state, DispatchTable& dispatchTable)
            {
                auto transitions = flatten_transition_table(state);

                bh::for_each(transitions, [&](auto transition){
                    addDispatchTableEntry(transition, dispatchTable);
                    addDispatchTableEntryOfSubMachineExits(transition, dispatchTable);
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
                auto takeWrappedEvent = [](auto event){return event.typeid_;};
                auto takeEvent = [](auto event){return bh::typeid_(event);};
                auto eventId = bh::if_(is_event(event), takeWrappedEvent, takeEvent)(event);

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