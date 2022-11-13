#include "hsm/hsm.h"

#include <gtest/gtest.h>

namespace {

using namespace ::testing;

// Events
struct e1 {
};
struct e2 {
};

struct SubInitial{
};

// States
struct Initial {

    /***
    * Initial states with transition table are not allowed
    *     
    */
    // static constexpr auto make_transition_table()
    // {
    //     // clang-format off
    //     return hsm::transition_table(* hsm::state<SubInitial> + hsm::event<e2> = hsm::state<SubInitial>);
    //     // clang-format on
    // }

    /***
    * Initial states with internal transition table are not allowed
    *     
    */
    // static constexpr auto make_internal_transition_table()
    // {
    //     // clang-format off
    //     return hsm::transition_table(
    //         + (hsm::event<e1>)
    //     );
    //     // clang-format on
    // }    
};
struct S2 {
};

struct MainState {
    static constexpr auto make_transition_table()
    {
        // clang-format off
        return hsm::transition_table(
                * hsm::state<Initial> + hsm::event<e1> = hsm::state<Initial>
        );
        // clang-format on
    }
};
}

/***
* Currently when the initial state has sub statemachine then this sub statemachine is not entered in the beginning.
* The sub statemachine is entered if you transition to the initial state via an event.
*/
TEST(InitialState, shouldStartInInitial)
{
    hsm::sm<MainState> sm;
    ASSERT_TRUE(sm.is(hsm::state<Initial>));
    sm.process_event(e1 {});
    ASSERT_TRUE(sm.is(hsm::state<Initial>));
}