#include "hsm/hsm.h"

#include <gtest/gtest.h>

#include <iostream>


// const auto collectStates = [](auto transitionTable){
//     return to<tuple_tag>(fold_left(transitionTable,  make_set(), [](auto states, auto row){
//         return insert(insert(states, front(row)), back(row));
//     }));
// };

// const auto collectEvents = [](auto transitionTable){
//     return to<tuple_tag>(fold_left(transitionTable,  make_set(), [](auto events, auto row){
//         return insert(events, at_c<1>(row));
//     }));
// };

// template <typename T>
// std::string const& name_of(basic_type<T>) {
//     static std::string name = boost::core::demangle(typeid(T).name());
//     return name;
// }

// const auto getIdx = [](auto indexMap, auto type) -> StateIdx {
//     return find(indexMap, type).value();
// };

// const auto printTypeName = [](const auto& type) {
//         std::cout << name_of(type) << std::endl;
// };

// const auto processEvent= [](auto dispatchTable, auto currentState, auto event) -> StateIdx{
//        return dispatchTable[currentState][event];
// };

// const auto makeIndexMap = [](auto tuple){
//     return to<map_tag>(second(fold_left(tuple, make_pair(int_c<0>, make_tuple()), [](auto acc, auto element){
//         auto i = first(acc);        
//         auto tuple = second(acc);        
//         auto inc = plus(i, int_c<1>);

//         return make_pair(inc, append(tuple, make_pair(element, i)));
//     })));
// };

// const auto makeDispatchTable = [](auto transitionTable, auto statesMap, auto eventsMap){
//     std::array<std::map<EventIdx, StateIdx>, length(collectStates(transitionTable))> dispatchTable;

//     for_each(transitionTable, [&](auto row){
//         auto from = getIdx(statesMap, front(row));
//         auto to = getIdx(statesMap, back(row));
//         auto with = getIdx(eventsMap, at_c<1>(row));

//         dispatchTable[from][with] = to;
//     });
//     return dispatchTable;
// };

// const auto is = [](auto statesMap, StateIdx currentState, auto state){
//     return currentState == getIdx(statesMap, state);
// };

// TEST(HsmTests, HsmTest){
    
//     auto transitionTable = 
//         make_tuple(
//             make_tuple(type<S1>{}, type<e1>{}, type<g1>{}, type<a1>{}, type<S2>{}), 
//             make_tuple(type<S1>{}, type<e2>{}, type<g1>{}, type<a1>{}, type<S3>{}), 
//             make_tuple(type<S2>{}, type<e2>{}, type<g1>{}, type<a1>{}, type<S1>{})
//         );

//     auto states = collectStates(transitionTable);
//     auto events = collectEvents(transitionTable);
//     for_each(states, printTypeName);
//     for_each(events, printTypeName);


//     auto statesMap = makeIndexMap(states);
//     auto eventsMap = makeIndexMap(events);

//     auto initialState = type<S1>{};
//     StateIdx currentState = getIdx(statesMap, initialState);
//     EventIdx event = getIdx(eventsMap, type<e1>{});

//     auto dispatchTable = makeDispatchTable(transitionTable, statesMap, eventsMap);

//     currentState = processEvent(dispatchTable, currentState, event);
//     ASSERT_TRUE(is(statesMap, currentState, type<S2>{}));

//     currentState = processEvent(dispatchTable, currentState, getIdx(eventsMap, type<e2>{}));
//     ASSERT_TRUE(is(statesMap, currentState, type<S1>{}));

//     currentState = processEvent(dispatchTable, currentState, getIdx(eventsMap, type<e2>{}));
//     ASSERT_TRUE(is(statesMap, currentState, type<S3>{}));

// }


// States
struct S1{};
struct S2{};
struct S3{};

// Events
struct e1{};
struct e2{};

// Guards
struct g1{};

// Actions
struct a1{};

using namespace ::testing;
using namespace boost::hana;

class HsmTests : public Test {
public:
    auto make_transition_table(){
        return boost::hana::make_tuple(
            boost::hana::make_tuple(type<S1>{}, type<e1>{}, type<g1>{}, type<a1>{}, type<S2>{}), 
            boost::hana::make_tuple(type<S1>{}, type<e2>{}, type<g1>{}, type<a1>{}, type<S3>{}), 
            boost::hana::make_tuple(type<S2>{}, type<e2>{}, type<g1>{}, type<a1>{}, type<S1>{})
        );
    }

    auto initialState(){
        return  type<S1>{};
    }    
};

TEST_F(HsmTests, should_start_in_initial_state){
    hsm::Sm sm(make_transition_table(), initialState());
    ASSERT_TRUE(sm.is(initialState()));
}

TEST_F(HsmTests, should_process_event){
    hsm::Sm sm(make_transition_table(), initialState());
    ASSERT_TRUE(sm.is(type<S1>{}));

    sm.process_event(type<e1>{});
    ASSERT_TRUE(sm.is(type<S2>{}));
}
