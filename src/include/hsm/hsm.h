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
            
    const auto states_impl = [](auto transitionTable){
        return to<tuple_tag>(fold_left(transitionTable,  make_set(), [](auto states, auto row){
            return insert(insert(states, front(row)), back(row));
        }));
    };

    template <class TransitionTable, class InitialState>    
    class Sm
    {
        TransitionTable m_transitionTable;
        std::array<std::map<EventIdx, StateIdx>, length(states_impl(TransitionTable{}))> m_dispatchTable;
        StateIdx m_currentState;

        public:
            Sm(TransitionTable transitionTable, InitialState inititalState) : m_transitionTable(transitionTable), m_currentState(getIdx(makeIndexMap(states()), inititalState))
            {
                makeDispatchTable();
            }

            template <class T>
            auto process_event(T event)
            {
                m_currentState = m_dispatchTable[m_currentState][getIdx(makeIndexMap(events()), event)];
            }

            template <class T>
            auto is(T state) -> bool {
                return m_currentState == getIdx(makeIndexMap(states()), state);
            };

        private:
            constexpr auto states(){
                return states_impl(m_transitionTable);
            }

            auto events(){
                return to<tuple_tag>(fold_left(m_transitionTable,  make_set(), [](auto events, auto row){
                    return insert(events, at_c<1>(row));
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

                for_each(m_transitionTable, [&](auto row){
                    auto from = getIdx(statesMap, front(row));
                    auto to = getIdx(statesMap, back(row));
                    auto with = getIdx(eventsMap, at_c<1>(row));

                    m_dispatchTable[from][with] = to;
                });
            }

            template <class T, class B>
            auto getIdx(T map, B type) -> Idx {
                return find(map, type).value();
            }
    };


}