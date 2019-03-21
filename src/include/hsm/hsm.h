#pragma once

#include <boost/hana.hpp>

#include <cstdint>
#include <map>
#include <array>

namespace hsm {

    using namespace boost::hana; 
    using Idx = std::uint16_t;   
    using StateIdx = Idx;
    using EventIdx = Idx;

    auto has_transition_table = is_valid([](auto&& state) -> decltype(state.make_transition_table()) { });

    template<class T>
    constexpr auto subStates(T&& state);

    class S1{};

    const auto collect_states = [](auto state){
        return to<tuple_tag>(to<set_tag>(fold_left(state.make_transition_table(),  make_tuple(), [](auto const& states, auto row){
            return concat(append(append(states, typeid_(front(row))), typeid_(back(row))), subStates(back(row)));    
        })));
    };

    template<class T>
    constexpr auto subStates(T&& state){
        return if_(has_transition_table(state),
            [](auto& stateWithTransitionTable){ return collect_states(stateWithTransitionTable);},
            [](auto&){ return make_tuple();})(state);
    };        

    template <class State>    
    class Sm
    {
        std::array<std::map<EventIdx, StateIdx>, length(collect_states(State{}))> m_dispatchTable;
        StateIdx m_currentState;
        StateIdx m_currentParentState;

        public:
            Sm() : m_currentState(getStateIdx(inititalState())), m_currentParentState(getStateIdx(rootState()))
            {
                makeDispatchTable();
            }

            template <class T>
            auto process_event(T event)
            {
                m_currentState = m_dispatchTable[m_currentState].at(getEventIdx(event));
            }

            template <class T>
            auto is(T state) -> bool {
                return m_currentState == getStateIdx(state);
            };

            template <class T, class B>
            auto is(T parentState, B state) -> bool {
                return m_currentParentState == getStateIdx(parentState) && m_currentState == getStateIdx(state);
            };

        private:
            auto transitionTable(){
                return State{}.make_transition_table();    
            }

            auto inititalState(){
                return State{}.initial_state();    
            }

            auto rootState(){
                return type<State>{};    
            }

            constexpr auto states(){
                return append(collect_states(State{}), type<State>{});
            }

            auto events(){
                return to<tuple_tag>(fold_left(transitionTable(),  make_set(), [](auto events, auto row){
                    return insert(events, typeid_(at_c<1>(row)));
                }));
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

            auto makeDispatchTable(){
                auto statesMap = makeIndexMap(states());
                auto eventsMap = makeIndexMap(events());

                for_each(transitionTable(), [&](auto row){
                    auto from = getStateIdx(front(row));
                    auto to = getStateIdx(back(row));
                    auto with = getEventIdx(at_c<1>(row));

                    m_dispatchTable[from][with] = to;
                });
            }

            template <class T>
            auto getStateIdx(T state){
                return getIdx(makeIndexMap(states()), typeid_(state));
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