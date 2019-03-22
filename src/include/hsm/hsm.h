#pragma once

#include <boost/hana.hpp>

#include <cstdint>
#include <map>
#include <array>
#include <iostream>

namespace hsm {

    using namespace boost::hana; 
    using Idx = std::uint16_t;   
    using StateIdx = Idx;
    using EventIdx = Idx;

    auto has_transition_table = is_valid([](auto&& state) -> decltype(state.make_transition_table()) { });

    template<class T>
    constexpr auto collect_sub_states(T&& state);

    template<class T>
    constexpr auto collect_sub_events(T&& state);

    const auto collect_states_recursive = [](auto state){
        return to<tuple_tag>(to<set_tag>(fold_left(state.make_transition_table(),  make_tuple(), [](auto const& states, auto row){
            return concat(append(append(states, typeid_(front(row))), typeid_(back(row))), collect_sub_states(back(row)));    
        })));
    };

    template<class T>
    constexpr auto collect_sub_states(T&& state){
        return if_(has_transition_table(state),
            [](auto& stateWithTransitionTable){ return collect_states_recursive(stateWithTransitionTable);},
            [](auto&){ return make_tuple();})(state);
    };        

    const auto collect_states = [](auto state){
        return to<tuple_tag>(to<set_tag>(fold_left(state.make_transition_table(),  make_tuple(), [](auto const& states, auto row){
            return append(append(states, typeid_(front(row))), typeid_(back(row)));    
        })));
    };    

    const auto collect_event_recursive = [](auto state){
        return to<tuple_tag>(to<set_tag>(fold_left(state.make_transition_table(),  make_tuple(), [](auto events, auto row){
              return concat(append(events, typeid_(at_c<1>(row))), collect_sub_events(back(row)));
        })));
    };

    template<class T>
    constexpr auto collect_sub_events(T&& state){
        return if_(has_transition_table(state),
            [](auto& stateWithTransitionTable){ return collect_event_recursive(stateWithTransitionTable);},
            [](auto&){ return make_tuple();})(state);
    };  

    const auto collect_events = [](auto state){
        return to<tuple_tag>(to<set_tag>(fold_left(state.make_transition_table(),  make_tuple(), [](auto events, auto row){
              return append(events, typeid_(at_c<1>(row)));
        })));
    };

    const auto collect_parent_states2 = [](auto state){
        return to<tuple_tag>(to<set_tag>(fold_left(state.make_transition_table(),  make_tuple(), [](auto const& states, auto row){
            auto state = back(row);
            auto subParentStates = if_(has_transition_table(state),
                [&states](auto& stateWithTransitionTable){ return append(collect_parent_states2(stateWithTransitionTable), typeid_(stateWithTransitionTable));},
                [&states](auto&){ return states;})(state);
            return concat(states, subParentStates);
        })));
    };

    const auto collect_parent_states = [](auto state){
        return append(collect_parent_states2(state), typeid_(state));
    };

    template <class State>    
    class Sm
    {
        std::array<std::map<StateIdx, std::map<EventIdx, std::pair<StateIdx, StateIdx>>>, length(collect_parent_states(State{}))> m_dispatchTable;
        StateIdx m_currentState;
        StateIdx m_currentParentState;

        public:
            Sm() : m_currentState(getStateIdx(inititalState())), m_currentParentState(getParentStateIdx(rootState()))
            {
                makeDispatchTable(rootState());
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
                return append(collect_states_recursive(rootState()), type<State>{});
            }

            auto events(){
                return collect_event_recursive(rootState());
            }

            template <class T>
            auto makeIndexMap(T tuple){
                return to<map_tag>(second(fold_left(tuple, make_pair(int_c<0>, make_tuple()), [](auto acc, auto element){
                    auto i = first(acc);        
                    auto tuple = second(acc);        
                    auto inc = plus(i, int_c<1>);

                    return make_pair(inc, append(tuple, make_pair(element, i)));
                })));
            }

            template <class T>
            auto makeDispatchTable(T state){
                auto parentStatesMap = makeIndexMap(parentStates());
                auto statesMap = makeIndexMap(states());
                auto eventsMap = makeIndexMap(events());

                auto fromParent = getParentStateIdx(state);

                for_each(state.make_transition_table(), [&](auto row){
                    auto from = getStateIdx(front(row));

                    auto to = if_(has_transition_table(back(row))
                        ,[this](auto state){ return getStateIdx(state.initial_state()); }
                        ,[this](auto state){ return getStateIdx(state);})(back(row));

                    auto toParent = if_(has_transition_table(back(row))
                        ,[this](auto state){ return getParentStateIdx(state);}
                        ,[this, &state](auto){ return getParentStateIdx(state);})(back(row));

                    auto with = getEventIdx(at_c<1>(row));

                    m_dispatchTable[fromParent][from][with] = std::make_pair(toParent, to);

                    // Add dispatch table of sub states                    
                    if_(has_transition_table(back(row))
                        ,[this](auto state){ makeDispatchTable(state);}
                        ,[](auto){})(back(row));

                    // Add dispatch table of sub states exits
                    if_(has_transition_table(front(row))
                        ,[this, with, toParent, to](auto parentState){ 
                            auto states = collect_states(parentState);

                            for_each(states, [this, parentState, with, toParent, to](auto state){
                                auto fromParent = getParentStateIdx(parentState);
                                auto from = getStateIdx(state);
                                m_dispatchTable[fromParent][from][with] = std::make_pair(toParent, to);
                            });
                            
                            makeDispatchTable(parentState);}
                        ,[](auto){})(front(row));
                    
                });
            }

            template <class T>
            auto getStateIdx(T state){
                return getIdx(makeIndexMap(states()), typeid_(state));
            }

            template <class T>
            auto getParentStateIdx(T parentState){
                return getIdx(makeIndexMap(parentStates()), typeid_(parentState));
            }

            template <class T>
            auto getEventIdx(T event){
                return getIdx(makeIndexMap(events()), typeid_(event));
            }

            template <class T, class B>
            auto getIdx(T map, B type) -> Idx {
                return find(map, type).value();
            }
    };


}